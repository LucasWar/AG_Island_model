#include "algoritmoGenetico.h"
#include "cvrpData.h"
#include "TypesUtils.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>

vectorIndiviudos GeneticAlgorithm::selecionarMigrantesUnicos(const vectorIndiviudos &populacao, std::mt19937 &gerador) {
    vectorIndiviudos selecionados;
    std::unordered_set<size_t> hashesVistos;

    std::vector<int> indices(populacao.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), gerador);

    for (int idx : indices) {
        const auto &ind = populacao[idx];
        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            hashesVistos.insert(h);
            selecionados.push_back(ind);
        }
        if (selecionados.size() >= tamMigracao) break;
    }
    return selecionados;
}

void GeneticAlgorithm::migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados) {
    int k = selecionados.size();
    if (k == 0 || populacao.empty()) return;

    if (k > populacao.size()) k = populacao.size();

    // Ordena população para trazer os piores para frente
    std::partial_sort(populacao.begin(), populacao.begin() + k, populacao.end(),
                      [](const Individuo &a, const Individuo &b){ return a.fitness < b.fitness; });

    for (int i = 0; i < k; ++i)
        populacao[i] = selecionados[i];
}

vectorIndiviudos GeneticAlgorithm::selecionarMigrantes_Torneio(const vectorIndiviudos &populacao, std::mt19937 &gerador, int tamanhoTorneio) {
    if (populacao.empty()) {
        return {};
    }

    vectorIndiviudos selecionados;
    std::unordered_set<size_t> hashesVistos;
    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);
    const int LIMITE_TENTATIVAS_SEM_SUCESSO = populacao.size() * 2;
    int tentativasSemSucesso = 0;

    while (selecionados.size() < tamMigracao && selecionados.size() < populacao.size()) {
        if (tentativasSemSucesso > LIMITE_TENTATIVAS_SEM_SUCESSO) {
            break; 
        }

        int indiceVencedor = -1;
        // Iniciar com um valor muito alto para garantir que o primeiro seja selecionado
        double melhorFitness = std::numeric_limits<double>::max(); 

        for (int i = 0; i < tamanhoTorneio; ++i) {
            int indiceCompetidor = dist(gerador);
            // CONDIÇÃO CORRIGIDA: usa '<' para minimização
            if (populacao[indiceCompetidor].fitness < melhorFitness) {
                melhorFitness = populacao[indiceCompetidor].fitness;
                indiceVencedor = indiceCompetidor;
            }
        }

        if (indiceVencedor != -1) {
            const auto &vencedor = populacao[indiceVencedor];
            size_t h = hashGenes(vencedor.genes);
            
            if (hashesVistos.find(h) == hashesVistos.end()) {
                hashesVistos.insert(h);
                selecionados.push_back(vencedor);
                tentativasSemSucesso = 0;
            } else {
                tentativasSemSucesso++;
            }
        } else {
            tentativasSemSucesso++;
        }
    }
    return selecionados;
}


void GeneticAlgorithm::realizarMigracao_Aprimorada(vetorIslands &islands) {
    const int TAMANHO_TORNEIO = 3;

    if (islands.size() < 2) return;

    // É mais eficiente criar uma lista de migrantes por ilha primeiro
    std::vector<vectorIndiviudos> migrantesPorIlha(islands.size());

    for (size_t i = 0; i < islands.size(); ++i) {
        // Seleciona os melhores para migrar (usando a função corrigida)
        migrantesPorIlha[i] = selecionarMigrantes_Torneio(islands[i].populacao, islands[i].geradorlocal, TAMANHO_TORNEIO);
    }

    for (size_t i = 0; i < islands.size(); ++i) {
        Island &ilhaOrigem = islands[i];
        for (int vizinhoId : ilhaOrigem.vizinhos) {
            auto it_destino = std::find_if(islands.begin(), islands.end(),
                                           [vizinhoId](const Island &il) { return il.idIlha == vizinhoId; });
            
            if (it_destino != islands.end()) {
                Island &ilhaDestino = *it_destino;
                const auto& migrantes = migrantesPorIlha[i]; // Migrantes da ilha de origem

                if (migrantes.empty()) continue;

                // Ordena a população de destino para que os PIORES fiquem no final
                std::sort(ilhaDestino.populacao.begin(), ilhaDestino.populacao.end(),
                          [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });
                
                // Cria um hashset dos genes existentes para evitar duplicatas
                std::unordered_set<size_t> hashesDestino;
                for(const auto& ind : ilhaDestino.populacao) {
                    hashesDestino.insert(hashGenes(ind.genes));
                }

                int indicePior = ilhaDestino.populacao.size() - 1;
                
                // Itera sobre os migrantes e substitui os piores indivíduos
                for (const auto& migrante : migrantes) {
                    // Se o pior indivíduo já foi substituído, paramos
                    if (indicePior < 0) break;

                    size_t hMigrante = hashGenes(migrante.genes);
                    if (hashesDestino.find(hMigrante) == hashesDestino.end()) {
                        // Substitui o pior, atualiza o hashset e move para o próximo pior
                        hashesDestino.erase(hashGenes(ilhaDestino.populacao[indicePior].genes));
                        ilhaDestino.populacao[indicePior] = migrante;
                        hashesDestino.insert(hMigrante);
                        indicePior--;
                    }
                }
            }
        }
    }
}