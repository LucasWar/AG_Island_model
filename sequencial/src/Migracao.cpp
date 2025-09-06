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
    std::partial_sort(populacao.begin(), populacao.begin() + k, populacao.end(),
                      [](const Individuo &a, const Individuo &b){ return a.fitness > b.fitness; });

    for (int i = 0; i < k; ++i)
        populacao[i] = selecionados[i];
}

// void GeneticAlgorithm::realizarMigracao(vetorIslands &islands) {
//     for (auto &ilhaOrigem : islands) {
//         auto selecionados = selecionarMigrantesUnicos(ilhaOrigem.populacao, ilhaOrigem.geradorlocal);
//         for (int vizinhoId : ilhaOrigem.vizinhos) {
//             auto it = std::find_if(islands.begin(), islands.end(),
//                                      [vizinhoId](const Island &il){ return il.idIlha == vizinhoId; });
//             if (it != islands.end())
//                 migracaoPopulacao(it->populacao, selecionados);
//         }
//     }
// }



vectorIndiviudos GeneticAlgorithm::selecionarMigrantes_Torneio(const vectorIndiviudos &populacao, std::mt19937 &gerador, int tamanhoTorneio) {
    if (populacao.empty()) {
        return {};
    }

    vectorIndiviudos selecionados;
    std::unordered_set<size_t> hashesVistos;

    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);

    // Variáveis de controle para evitar loop infinito
    const int LIMITE_TENTATIVAS_SEM_SUCESSO = populacao.size() * 2; // Um limite razoável
    int tentativasSemSucesso = 0;

    while (selecionados.size() < tamMigracao && selecionados.size() < populacao.size()) {
        
        // Condição de saída de emergência
        if (tentativasSemSucesso > LIMITE_TENTATIVAS_SEM_SUCESSO) {
            // Opcional: imprimir um aviso de que não foi possível encontrar a quantidade desejada de migrantes únicos.
            // std::cout << "Aviso: Não foi possível encontrar " << tamMigracao << " migrantes únicos. Retornando " << selecionados.size() << ".\n";
            break; 
        }

        int indiceVencedor = -1;
        double melhorFitness = -1.0; 

        for (int i = 0; i < tamanhoTorneio; ++i) {
            int indiceCompetidor = dist(gerador);
            // IMPORTANTE: Adicionar verificação para evitar acesso fora dos limites se a população for pequena.
            if (populacao[indiceCompetidor].fitness > melhorFitness) {
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
                tentativasSemSucesso = 0; // Reseta o contador no sucesso
            } else {
                tentativasSemSucesso++; // Incrementa o contador na falha
            }
        } else {
             tentativasSemSucesso++; // Se nenhum vencedor for encontrado (improvável, mas seguro)
        }
    }
    return selecionados;
}


void GeneticAlgorithm::realizarMigracao_Aprimorada(vetorIslands &islands) {
    const int TAMANHO_TORNEIO = 3; // Um bom valor padrão

    if (islands.size() < 2) return; // Migração não faz sentido com menos de 2 ilhas

    for (auto &ilhaOrigem : islands) {
        // Para cada vizinho, selecionamos um novo e único conjunto de migrantes
        for (int vizinhoId : ilhaOrigem.vizinhos) {
            
            // Passo 1: Selecionar migrantes de alta qualidade da ilha de origem
            auto selecionados = selecionarMigrantes_Torneio(ilhaOrigem.populacao, ilhaOrigem.geradorlocal, TAMANHO_TORNEIO);
            
            if (selecionados.empty()) continue; // Nenhum migrante selecionado

            // Passo 2: Encontrar a ilha de destino
            auto it_destino = std::find_if(islands.begin(), islands.end(),
                                       [vizinhoId](const Island &il) { return il.idIlha == vizinhoId; });
            
            // Passo 3: Inserir os migrantes na ilha de destino (substituindo os piores)
            if (it_destino != islands.end()) {
                migracaoPopulacao(it_destino->populacao, selecionados);
            }
        }
    }
    
}