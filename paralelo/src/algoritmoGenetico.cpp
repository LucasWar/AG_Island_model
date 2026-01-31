#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "cvrpData.h"
#include "Fitness.h"
#include "Utils.h"
#include "ilha.h"
#include <chrono>
#include <iostream>
#include <algorithm>
#include <omp.h>
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

    #pragma omp parallel shared(ilhas, numGeracoes, melhor, numInslands, auxNumGerSemEvo, numGerSemEvo, dataCVRP, std::cout) num_threads(numInslands) 
    {

        std::uniform_real_distribution<double> distLocal(0.0, 1.0);

        for (int geracao = 0; geracao < numGeracoes; ++geracao) {


            //EXECUÇÃO PARALELA DAS ILHAS
            #pragma omp for schedule(static)
            for (int i = 0; i < (int)ilhas.size(); i++) {
                //printf("Thread %d -> executando ilha %d\n", omp_get_thread_num(), i);
                executarGeracao(ilhas[i], distLocal);
            }



            //MIGRAÇÃO (executada apenas por 1 thread)
            #pragma omp single
            {
                if (numInslands > 1) {
                    std::uniform_real_distribution<double> probMigracao(0.0, 1.0);

                    for (auto &ilha : ilhas) {
                        if (probMigracao(ilha.geradorlocal) < 0.15) {
                            realizarMigracao_Aprimorada(ilhas);
                            break;
                        }
                    }
                }
            }



            //AVALIAÇÃO DO MELHOR INDIVÍDUO (apenas 1 thread)
            #pragma omp single
            {
                auto auxMelhor = melhorIndividuo(ilhas, geracao);

                if (auxMelhor.fitness < melhor.fitness) {
                    melhor = auxMelhor;
                    if (auxNumGerSemEvo > numGerSemEvo) {
                        numGerSemEvo = auxNumGerSemEvo;
                    }
                    auxNumGerSemEvo = 0;
                } else {
                    auxNumGerSemEvo++;
                }
            }
        }
    }
    auto fim = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracao = fim - inicio;
    int teste = dataCVRP.solucaoOtima;
    int duracao_s = std::chrono::duration_cast<std::chrono::seconds>(duracao).count();
    salvarResultados(melhor, teste, duracao_s, numGerSemEvo, melhor.genes, dataCVRP, numInslands,opcTopologia);
}





void GeneticAlgorithm::executarAlgoritmoTime() {

    int numGerSemEvo = 0;
    int auxNumGerSemEvo = 0;

    Topologia topologia;

    std::cout << "==================================================TOPOLOGIA=========================================================\n";
    auto ilhas = topologia.criarTopologia(
        opcTopologia, numInslands, seed, opcCrossover, opcSelecao
    );

    if (ilhas.empty()) {
        throw std::runtime_error("Erro: Nenhuma ilha foi criada. Verifique os parâmetros da topologia.");
    }

    std::cout << "Numero de individuos por ilha: " << tamPopulacao << std::endl;
    std::cout << "Preservação de elite: " << tamElite << std::endl;
    std::cout << "====================================================================================================================\n\n";

    std::cout << "===================================================EXECUCAO=========================================================\n\n";

    auto inicioExecucao = std::chrono::steady_clock::now();
    const int LIMITE_TEMPO = 200; // segundos

    std::cout << "Gerando população inicial..." << std::endl;
    gerarPopulacaoDiversificada(ilhas);
    std::cout << "População inicial gerada." << std::endl;

    Individuo melhor = melhorIndividuo(ilhas, 0);
    std::cout << "Melhor fitness inicial: " << melhor.fitness << std::endl;

    // 🔴 FLAG DE PARADA COMPARTILHADA
    bool parar = false;

    #pragma omp parallel shared(ilhas, melhor, numGerSemEvo, auxNumGerSemEvo, parar, dataCVRP) num_threads(numInslands)
    {
        std::uniform_real_distribution<double> distLocal(0.0, 1.0);
        int geracao = 0;

        while (true) {

            // ================= CONTROLE DE TEMPO =================
            #pragma omp single
            {
                auto agora = std::chrono::steady_clock::now();
                auto tempoDecorrido = std::chrono::duration_cast<std::chrono::seconds>(
                    agora - inicioExecucao
                ).count();

                if (tempoDecorrido >= LIMITE_TEMPO) {
                    parar = true;
                }
            }

            // 🔒 GARANTE QUE TODAS VEJAM "parar"
            #pragma omp barrier
            if (parar) break;

            // ================= EXECUÇÃO DAS ILHAS =================
            #pragma omp for schedule(static)
            for (int i = 0; i < (int)ilhas.size(); i++) {
                executarGeracao(ilhas[i], distLocal);
            }

            // ================= MIGRAÇÃO =================
            #pragma omp single
            {
                if (numInslands > 1) {
                    std::uniform_real_distribution<double> probMigracao(0.0, 1.0);
                    for (auto &ilha : ilhas) {
                        if (probMigracao(ilha.geradorlocal) < 0.15) {
                            realizarMigracao_Aprimorada(ilhas);
                            break;
                        }
                    }
                }
            }

            // ================= MELHOR INDIVÍDUO =================
            #pragma omp single
            {
                auto auxMelhor = melhorIndividuo(ilhas, geracao);

                if (auxMelhor.fitness < melhor.fitness) {
                    melhor = auxMelhor;
                    auxNumGerSemEvo = 0;
                } else {
                    auxNumGerSemEvo++;
                }

                numGerSemEvo = std::max(numGerSemEvo, auxNumGerSemEvo);
            }

            geracao++;
        }
    }

    auto fim = std::chrono::steady_clock::now();
    std::chrono::duration<double> duracao = fim - inicioExecucao;

    int duracao_s = std::chrono::duration_cast<std::chrono::seconds>(duracao).count();
    int solucaoOtima = dataCVRP.solucaoOtima;

    salvarResultados(
        melhor,
        solucaoOtima,
        duracao_s,
        numGerSemEvo,
        melhor.genes,
        dataCVRP,
        numInslands,
        opcTopologia
    );
}


void GeneticAlgorithm::executarAlgoritmoTimeSequencial() {

    int numGerSemEvo = 0;
    int auxNumGerSemEvo = 0;

    Topologia topologia;

    std::cout << "==================================================TOPOLOGIA=========================================================\n";
    auto ilhas = topologia.criarTopologia(
        opcTopologia, numInslands, seed, opcCrossover, opcSelecao
    );

    if (ilhas.empty()) {
        throw std::runtime_error("Erro: Nenhuma ilha foi criada. Verifique os parâmetros da topologia.");
    }

    std::cout << "Numero de individuos por ilha: " << tamPopulacao << std::endl;
    std::cout << "Preservação de elite: " << tamElite << std::endl;
    std::cout << "====================================================================================================================\n\n";

    std::cout << "===================================================EXECUCAO=========================================================\n\n";

    auto inicioExecucao = std::chrono::steady_clock::now();
    const int LIMITE_TEMPO = 200; // segundos

    std::cout << "Gerando população inicial..." << std::endl;
    gerarPopulacaoDiversificada(ilhas);
    std::cout << "População inicial gerada." << std::endl;

    Individuo melhor = melhorIndividuo(ilhas, 0);
    std::cout << "Melhor fitness inicial: " << melhor.fitness << std::endl;

    int geracao = 0;

    // ================= LOOP PRINCIPAL SEQUENCIAL =================
    while (true) {

        // ================= CONTROLE DE TEMPO =================
        auto agora = std::chrono::steady_clock::now();
        auto tempoDecorrido = std::chrono::duration_cast<std::chrono::seconds>(
            agora - inicioExecucao
        ).count();

        if (tempoDecorrido >= LIMITE_TEMPO) {
            break;
        }
        std::uniform_real_distribution<double> distLocal(0.0, 1.0);
        // ================= EXECUÇÃO DAS ILHAS =================
        for (int i = 0; i < (int)ilhas.size(); i++) {
            executarGeracao(ilhas[i], distLocal);
        }

        // ================= MIGRAÇÃO =================
        if (numInslands > 1) {
            std::uniform_real_distribution<double> probMigracao(0.0, 1.0);
            for (auto &ilha : ilhas) {
                if (probMigracao(ilha.geradorlocal) < 0.15) {
                    realizarMigracao_Aprimorada(ilhas);
                    break;
                }
            }
        }

        // ================= MELHOR INDIVÍDUO =================
        auto auxMelhor = melhorIndividuo(ilhas, geracao);

        if (auxMelhor.fitness < melhor.fitness) {
            melhor = auxMelhor;
            auxNumGerSemEvo = 0;
        } else {
            auxNumGerSemEvo++;
        }

        numGerSemEvo = std::max(numGerSemEvo, auxNumGerSemEvo);

        geracao++;
    }

    auto fim = std::chrono::steady_clock::now();
    std::chrono::duration<double> duracao = fim - inicioExecucao;

    int duracao_s = std::chrono::duration_cast<std::chrono::seconds>(duracao).count();
    int solucaoOtima = dataCVRP.solucaoOtima;

    salvarResultados(
        melhor,
        solucaoOtima,
        duracao_s,
        numGerSemEvo,
        melhor.genes,
        dataCVRP,
        numInslands,
        opcTopologia
    );
}
