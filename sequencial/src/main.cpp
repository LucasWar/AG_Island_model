#include "algoritmoGenetico.h"
#include "Utils.h"
#include <chrono>
#include <iostream>
#include <omp.h>
#include <windows.h>

#define ARQUIVO_ENTRADA "entradas/X-n143-k7.txt"
#define NUMERO_GERACOES 3000
#define PROB_MUTACAO    0.2
#define TAM_POPULCAO    63
#define TAM_ELITE       0.08
#define NUM_ISLANDS     32
#define FREQ_MIGRACAO   350
#define NUM_MIGRACAO    0.05
#define TOPOLOGIA       "Malha"    
#define SELECAO         "Torneio"  
#define CROSSOVER       "PMX"      

int main() {
    SetConsoleOutputCP(CP_UTF8);
    int numExecucoes = 10;
    int seedSize = 18;

    for(int i = 0; i < numExecucoes; i++){
        std::uint64_t seed = lerArquivoSeed("pi.txt",i*seedSize,(i + 1)*seedSize);
        std::cout << "Execucao N° " << i + 1 << " SEED: " << seed << std::endl;
        GeneticAlgorithm ga(
            NUMERO_GERACOES, 
            PROB_MUTACAO, 
            TAM_POPULCAO, 
            TAM_ELITE, 
            ARQUIVO_ENTRADA,
            seed,
            NUM_ISLANDS,
            NUM_MIGRACAO,
            TOPOLOGIA,
            FREQ_MIGRACAO,
            CROSSOVER,
            SELECAO
        );
        
        ga.executarAlgoritmo();
        std::cout << std::endl;
        std::cout << std::endl;
    }
   
    return 0;
}
