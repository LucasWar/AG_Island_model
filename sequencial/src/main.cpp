#include "algoritmoGenetico.h"
#include "Utils.h"
#include <chrono>
#include <iostream>
#include <omp.h>

#define ARQUIVO_ENTRADA "entradas/DANTZIG42.txt"
#define NUMERO_GERACOES 2000
#define PROB_MUTACAO    0.2
#define TAM_POPULCAO    120
#define TAM_ELITE       0.1
#define NUM_ISLANDS     5
int main() {
    int numExecucoes = 10;
    int seedSize = 18;

    for(int i = 0; i < numExecucoes; i++){
        std::uint64_t seed = lerArquivoSeed("pi.txt",i*seedSize,(i + 1)*seedSize);
        GeneticAlgorithm ga(NUMERO_GERACOES, PROB_MUTACAO, TAM_POPULCAO, TAM_ELITE, ARQUIVO_ENTRADA,seed,NUM_ISLANDS);
        auto inicio = std::chrono::high_resolution_clock::now();
        ga.executarAlgoritmo();
        auto fim = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duracao = fim - inicio;
        std::cout << "Semente enviada " << seed << std::endl;
        std::cout << "\nTempo de execucao: " << duracao.count() << " segundos\n";

    }
   
    return 0;
}
