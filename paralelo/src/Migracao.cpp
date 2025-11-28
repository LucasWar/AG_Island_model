#include "algoritmoGenetico.h"
#include "cvrpData.h"
#include "TypesUtils.h"
#include "Utils.h"
#include <iostream>
#include <algorithm>
#include <unordered_set>

// vectorIndiviudos GeneticAlgorithm::selecionarMigrantesUnicos(const vectorIndiviudos &populacao, std::mt19937 &gerador) {
//     vectorIndiviudos selecionados;
//     std::unordered_set<size_t> hashesVistos;

//     std::vector<int> indices(populacao.size());
//     std::iota(indices.begin(), indices.end(), 0);
//     std::shuffle(indices.begin(), indices.end(), gerador);

//     for (int idx : indices) {
//         const auto &ind = populacao[idx];
//         size_t h = hashGenes(ind.genes);
//         if (hashesVistos.find(h) == hashesVistos.end()) {
//             hashesVistos.insert(h);
//             selecionados.push_back(ind);
//         }
//         if (selecionados.size() >= tamMigracao) break;
//     }
    
//     return selecionados;
// }

// void GeneticAlgorithm::migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados) {
//     int k = selecionados.size();
//     if (k == 0 || populacao.empty()) return;

//     if (k > populacao.size()) k = populacao.size();

//     // Ordena população para trazer os piores para frente
//     std::partial_sort(populacao.begin(), populacao.begin() + k, populacao.end(),
//                       [](const Individuo &a, const Individuo &b){ return a.fitness < b.fitness; });

//     for (int i = 0; i < k; ++i)
//         populacao[i] = selecionados[i];
// }
vectorIndiviudos GeneticAlgorithm::selecionarMigrantesMelhores(const vectorIndiviudos &populacao, const double parcelaPop){
     if (populacao.empty()) {
        return {};
    }

    vectorIndiviudos copia = populacao;

    // Ordena apenas os n melhores (menor fitness) no início do vetor
    std::partial_sort(copia.begin(),
                      copia.begin() + (parcelaPop * tamPopulacao),
                      copia.end(),
                      [](const Individuo &a, const Individuo &b) {
                          return a.fitness < b.fitness; // Minimização
                      });

    // Retorna apenas os n melhores (já ordenados)
    return vectorIndiviudos(copia.begin(), copia.begin() + (parcelaPop * tamPopulacao));
    
}

vectorIndiviudos GeneticAlgorithm::selecionarMigrantes_Torneio(const vectorIndiviudos &populacao, std::mt19937 &gerador, int tamanhoTorneio, double parcelaPop) {
    if (populacao.empty()) {
        return {};
    }

    vectorIndiviudos selecionados;
    std::unordered_set<size_t> hashesVistos;
    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);
    const int LIMITE_TENTATIVAS_SEM_SUCESSO = populacao.size() * 2;
    int tentativasSemSucesso = 0;

    while (selecionados.size() < (tamMigracao*parcelaPop) && selecionados.size() < populacao.size()) {
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


void GeneticAlgorithm::realizarMigracao_Aprimorada(vetorIlhas &ilhas) {
    if (ilhas.size() < 2) return;

    const int TAMANHO_TORNEIO = 2;
    const double PROB_MIGRAR = 1.0; // 60% das ilhas migram a cada rodada

    // --- 1. Escolhe ilhas que migrarão nesta rodada ---
    auto ilhasMigrantes = escolherIlhasMigrantes(ilhas,PROB_MIGRAR);
    // --- 2. Seleciona migrantes de cada ilha ---
    auto migrantesPorIlha = coletarMigrantes(ilhas,ilhasMigrantes,TAMANHO_TORNEIO);
    
    
    // --- 3. Envia migrantes para os vizinhos ---
    enviarMigrantes(ilhas,ilhasMigrantes,migrantesPorIlha);
}


std::vector<size_t> GeneticAlgorithm::escolherIlhasMigrantes(vetorIlhas &ilhas, double probMigrar){
    std::vector<size_t> ilhasMigrantes;
    for (size_t i = 0; i < ilhas.size(); ++i) {
        std::uniform_real_distribution<double> prob(0.0, 1.0);
        if (prob(ilhas[i].geradorlocal) <= probMigrar)
            ilhasMigrantes.push_back(i);
    }

    return ilhasMigrantes;
}


std::vector<vectorIndiviudos> GeneticAlgorithm::coletarMigrantes(vetorIlhas &ilhas, const std::vector<size_t> &ilhasMigrantes,int tamTorneio){
    std::vector<vectorIndiviudos> migrantesPorIlha(ilhas.size());
    for (size_t i : ilhasMigrantes) {
        auto &ilha = ilhas[i];
        auto migrantesTorneio = selecionarMigrantes_Torneio(ilha.populacao, ilha.geradorlocal, tamTorneio,0.05);
        //auto migrantesMelhores = selecionarMigrantesMelhores(ilha.populacao,0.01);
        vectorIndiviudos migrantes;
        // Mantém apenas 50% dos melhores migrantes
        // std::sort(migrantes.begin(), migrantes.end(),
        //           [](const Individuo &a, const Individuo &b) {
        //               return a.fitness < b.fitness;
        //           });
        // if (migrantes.size() > 2)
        //     migrantes.resize(migrantes.size() / 2);

        //migrantes.reserve(migrantesTorneio.size() + migrantesMelhores.size());
        migrantes.insert(migrantes.end(), migrantesTorneio.begin(), migrantesTorneio.end());
        //migrantes.insert(migrantes.end(), migrantesMelhores.begin(), migrantesMelhores.end());
        migrantesPorIlha[i] = std::move(migrantes);
    }

    return migrantesPorIlha;
}

void GeneticAlgorithm::enviarMigrantes(vetorIlhas &ilhas, const std::vector<size_t> &ilhasMigrantes, const std::vector<vectorIndiviudos> &migrantesPorIlha){
    for (size_t i : ilhasMigrantes) {
        Ilha &origem = ilhas[i];
        const auto &migrantes = migrantesPorIlha[i];
        if (migrantes.empty()) continue;

        for (int vizinhoId : origem.vizinhos) {
            auto it_destino = std::find_if(ilhas.begin(), ilhas.end(),
                [vizinhoId](const Ilha &il) { return il.idIlha == vizinhoId; });

            if (it_destino == ilhas.end()) continue;
            Ilha &destino = *it_destino;

            // Ordena destino para proteger bons indivíduos
            std::sort(destino.populacao.begin(), destino.populacao.end(),
                      [](const Individuo &a, const Individuo &b) {
                          return a.fitness < b.fitness;
                      });

            std::unordered_set<size_t> hashDestino;
            for (const auto &ind : destino.populacao)
                hashDestino.insert(hashGenes(ind.genes));
            
            if(destino.populacao.empty()) continue;
            int idxSubst = destino.populacao.size() - 1; // substitui piores

            std::uniform_real_distribution<double> distLocal(0.0, 1.0);

            for (auto migrante : migrantes) {
                if (idxSubst < 0) break;
                size_t h = hashGenes(migrante.genes);
                if (hashDestino.find(h) == hashDestino.end()) {
                    // 30% dos migrantes sofrem mutação antes de entrar
                    if (distLocal(destino.geradorlocal) < 0.3)
                        mutacaoCVRP(migrante, destino.geradorlocal);

                    destino.populacao[idxSubst--] = std::move(migrante);
                    hashDestino.insert(h);
                }
            }
        }
    }
}