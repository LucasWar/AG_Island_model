#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "utils.h"
#include "cvrpData.h"
#include <iostream>
#include <algorithm>
#include <list>

GeneticAlgorithm::GeneticAlgorithm(int nGen, float pMut, int tPop, float nElite, const std::string& caminho,std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,int freqMigracao) 
    :seed(seed),numGeracoes(nGen), probMutacao(pMut), tamPopulacao(tPop),numInslands(numInslands),opcTopologia(opcTopologia),freqMigracao(freqMigracao), tamMigracao(static_cast<int>(std::floor(numMigracao * tamPopulacao))), tamElite(static_cast<int>(std::floor(nElite * tamPopulacao))){
    dataCVRP = lerArquivoVRP(caminho);
    tamIndividuo = dataCVRP.distancias.empty() ? 0 : dataCVRP.distancias.size(); 
}

Individuo GeneticAlgorithm::melhorIndividuo(vetorIslands &islands) {
    bool inicializado = false;
    Individuo melhor;

    for (auto &island : islands) {
        if (island.populacao.empty()) continue;

        auto it = std::min_element(island.populacao.begin(), island.populacao.end(),
                                     [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

        if (!inicializado || it->fitness < melhor.fitness) {
            melhor = *it;
            inicializado = true;
        }
    }

    if (!inicializado) throw std::runtime_error("Nenhuma população encontrada para avaliar.");
    return melhor;
}


void GeneticAlgorithm::gerarPopulacao(vetorIslands &islands){
    for(auto &island : islands){
        island.populacao.reserve(tamPopulacao);
        for(int i = 0; i < tamPopulacao; ){ 
            
            std::vector<int> clientes(tamIndividuo - 1);
            std::iota(clientes.begin(), clientes.end(), 1);
            std::shuffle(clientes.begin(), clientes.end(), island.geradorlocal);

            std::vector<int> genes;
            std::list<int> clientesNaoAtendidos(clientes.begin(), clientes.end());

            int veiculosUsados = 0;
            while(!clientesNaoAtendidos.empty() && veiculosUsados < dataCVRP.numVeiculos) {
                veiculosUsados++;
                genes.push_back(0); // Inicia uma nova rota
                double cargaAtual = 0.0;
                
                for(auto it = clientesNaoAtendidos.begin(); it != clientesNaoAtendidos.end(); ){
                    int clienteAtual = *it;
                    if(cargaAtual + dataCVRP.demandas[clienteAtual] <= dataCVRP.capacidade){
                        genes.push_back(clienteAtual);
                        cargaAtual += dataCVRP.demandas[clienteAtual];
                        it = clientesNaoAtendidos.erase(it);
                    } else {
                        ++it;
                    }
                }
                genes.push_back(0); // Fecha a rota atual
            }

            if (!clientesNaoAtendidos.empty()) {
                // Tenta gerar o indivíduo 'i' novamente se a permutação não foi viável
                continue;
            }

            Individuo ind(genes, calcularFitness(genes));
            island.populacao.push_back(ind);
            i++; // Indivíduo válido gerado, incrementa
        }
    }
}

void GeneticAlgorithm::executarAlgoritmo() {
    std::uniform_real_distribution<double> distLocal(0, 1);
    Topologia topologia;
    auto islands = topologia.criarTopologia(opcTopologia,numInslands,seed);

    std::cout << "Gerando população inicial..." << std::endl;
    // ALTERADO: Chamando a função de geração correta (agora renomeada)
    gerarPopulacao(islands);
    std::cout << "População inicial gerada." << std::endl;

    // ALTERADO: Descomentando o loop principal para executar o algoritmo
    Individuo melhor = melhorIndividuo(islands);
    std::cout << "Melhor fitness inicial: " << melhor.fitness << std::endl;

    for (int geracao = 0; geracao < numGeracoes; ++geracao) {
        for (auto &ilha : islands)
            executarGeracao(ilha, distLocal);

        if (geracao > 0 && geracao % freqMigracao == 0)
            realizarMigracao(islands);

        auto auxMelhor = melhorIndividuo(islands);
        if (auxMelhor.fitness < melhor.fitness) {
            melhor = auxMelhor;
            std::cout << "Geração " << geracao + 1 << " | Novo melhor fitness: " << melhor.fitness << std::endl;
        }
    }

    std::cout << "\n===================================" << std::endl;
    std::cout << "Evolução concluída!" << std::endl;
    std::cout << "Melhor fitness final: " << melhor.fitness << std::endl;
    std::cout << "Melhor solução encontrada: ";
    printVector(melhor.genes);
    std::cout << "===================================" << std::endl;
}