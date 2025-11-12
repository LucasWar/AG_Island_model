#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "cvrpData.h"
#include "Fitness.h"
#include "Utils.h"
#include "ilha.h"
#include <chrono>
#include <iostream>
#include <algorithm>

GeneticAlgorithm::GeneticAlgorithm(int nGen, float pMut, int tPop, float nElite, const std::string& caminho,std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,int freqMigracao,std::string opcCrossover,std::string opcSelecao) 
    :seed(seed),numGeracoes(nGen), probMutacao(pMut), tamPopulacao(tPop),numInslands(numInslands),opcTopologia(opcTopologia),opcCrossover(opcCrossover),opcSelecao(opcSelecao),freqMigracao(freqMigracao), tamMigracao(static_cast<int>(std::floor(numMigracao * tamPopulacao))), tamElite(static_cast<int>(std::floor(nElite * tamPopulacao))){
    dataCVRP = lerArquivoVRP(caminho);
    tamIndividuo = dataCVRP.distancias.empty() ? 0 : dataCVRP.distancias.size(); 
}

void GeneticAlgorithm::executarAlgoritmo() {
    //std::cout << "Frequencia de migração: "<< freqMigracao << std::endl;
    int numGerSemEvo = 0;
    int auxNumGerSemEvo = 0;

    std::uniform_real_distribution<double> distLocal(0, 1);
    Topologia topologia;
    std::cout << "==================================================TOPOLOGIA=========================================================" << std::endl;
    auto ilhas = topologia.criarTopologia(opcTopologia,numInslands,seed,opcCrossover,opcSelecao);
    if(ilhas.size() == 0){
        throw std::runtime_error("Erro: Nenhuma ilha foi criada. Verifique os parâmetros da topologia.");
    }
    std::cout << "Numero de individuos por ilha: "<< tamPopulacao << std::endl;
    std::cout << "Preservação de elite: "<< tamElite << std::endl;

    std::cout << "====================================================================================================================" <<  "\n" << std::endl;
    
    std::cout << "===================================================EXECUCAO=========================================================" <<  "\n" << std::endl;

    auto inicio = std::chrono::high_resolution_clock::now();
    std::cout << "Gerando população inicial..." << std::endl;
    gerarPopulacaoDiversificada(ilhas);
    std::cout << "População inicial gerada." << std::endl;

    Individuo melhor = melhorIndividuo(ilhas,0);
    std::cout << "Melhor fitness inicial: " << melhor.fitness << std::endl;

    for (int geracao = 0; geracao < numGeracoes; ++geracao) {
        // #pragma omp parallel for
        // for (auto &ilha : ilhas)
        //     if(ilha.reset == true){
        //         reiniciarPopulacoes(ilha,geracao);
        //         ilha.reset = false;
        //     }
        
        #pragma omp parallel for schedule(dynamic)
        for (auto &ilha : ilhas){
            executarGeracao(ilha, distLocal);
        }

        if(numInslands > 1){
            std::uniform_real_distribution<double> probMigracao(0.0, 1.0);
            for (auto &ilha : ilhas) {
                if (probMigracao(ilha.geradorlocal) < 0.15) { // 5% chance por geração
                    realizarMigracao_Aprimorada(ilhas);
                    break;
                }
            }
        }

        auto auxMelhor = melhorIndividuo(ilhas, geracao);
        if (auxMelhor.fitness < melhor.fitness) {
            melhor = auxMelhor;
            if (auxNumGerSemEvo > numGerSemEvo) {
                numGerSemEvo = auxNumGerSemEvo;
            }
            auxNumGerSemEvo = 0;
            //std::cout << "Geração " << geracao + 1 << " | Novo melhor fitness: " << melhor.fitness << std::endl;
        } else {
            auxNumGerSemEvo++;
        }

    }
    auto fim = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracao = fim - inicio;
    int teste = dataCVRP.solucaoOtima;
    int duracao_s = std::chrono::duration_cast<std::chrono::seconds>(duracao).count();
    salvarResultados(melhor, teste, duracao_s, numGerSemEvo, melhor.genes, dataCVRP, numInslands);
}