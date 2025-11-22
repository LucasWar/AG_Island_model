#include "algoritmoGenetico.h"
#include "Utils.h"
#include <chrono>
#include <iostream>
#include <windows.h>

#define ARQUIVO_ENTRADA "entradas/A-n80-k10.txt"
#define NUMERO_GERACOES 3000
#define PROB_MUTACAO    0.25
#define TAM_POPULCAO    2000
#define TAM_ELITE       0.08
#define NUM_ISLANDS     1
#define FREQ_MIGRACAO   5
#define NUM_MIGRACAO    0.05
#define TOPOLOGIA       "Malha"    
#define SELECAO         "Torneio"  
#define CROSSOVER       "RBX"      

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
