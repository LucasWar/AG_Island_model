#include "algoritmoGenetico.h"
#include "Utils.h"
#include <chrono>
#include <iostream>
#include <omp.h>

#define ARQUIVO_ENTRADA "entradas/B-n78-k10.txt"
#define NUMERO_GERACOES 2500
#define PROB_MUTACAO    0.2
#define TAM_POPULCAO    200
#define TAM_ELITE       0.15
#define NUM_ISLANDS     8
#define FREQ_MIGRACAO   150
#define NUM_MIGRACAO    0.25
#define TOPOLOGIA       "Malha" //["Malha","Anel","Aleatoria"]

int main() {
    int numExecucoes = 10;
    int seedSize = 18;

    for(int i = 0; i < numExecucoes; i++){
        std::uint64_t seed = lerArquivoSeed("pi.txt",i*seedSize,(i + 1)*seedSize);
        GeneticAlgorithm ga(NUMERO_GERACOES, PROB_MUTACAO, TAM_POPULCAO, TAM_ELITE, ARQUIVO_ENTRADA,seed,NUM_ISLANDS,NUM_MIGRACAO,TOPOLOGIA,FREQ_MIGRACAO);
        auto inicio = std::chrono::high_resolution_clock::now();
        ga.executarAlgoritmo();
        auto fim = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = fim - inicio;
        std::cout << "Semente enviada " << seed << std::endl;
        std::cout << "\nTempo de execucao: " << duracao.count() << " segundos\n";

    }
   
    return 0;
}
