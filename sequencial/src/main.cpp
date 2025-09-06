#include "algoritmoGenetico.h"
#include "Utils.h"
#include <chrono>
#include <iostream>
#include <omp.h>
#include <windows.h>

#define ARQUIVO_ENTRADA "entradas/E-n101-k8.txt"
#define NUMERO_GERACOES 3000
#define PROB_MUTACAO    0.2
#define TAM_POPULCAO    60
#define TAM_ELITE       0.05
#define NUM_ISLANDS     5
#define FREQ_MIGRACAO   40
#define NUM_MIGRACAO    0.2
#define TOPOLOGIA       "Malha"    
#define SELECAO         "Roleta"  
#define CROSSOVER       "PMX"      

int main() {
    SetConsoleOutputCP(CP_UTF8);
    int numExecucoes = 1;
    int seedSize = 18;

    for(int i = 0; i < numExecucoes; i++){
        std::cout << "Execucao N° " << i + 1 << std::endl;
        std::uint64_t seed = lerArquivoSeed("pi.txt",i*seedSize,(i + 1)*seedSize);
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
            CROSSOVER,
            FREQ_MIGRACAO);
        
        ga.executarAlgoritmo();
        std::cout << std::endl;
        std::cout << std::endl;
    }
   
    return 0;
}
