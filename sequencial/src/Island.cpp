#include "Island.h"
#include <iostream>
#include <random>

// definição do static
int Island::nextId = 1;

// construtor
Island::Island(int seed) : idIlha(nextId++), seed(seed), geradorlocal(idIlha + seed){} 

void Island::imprimirPopulacao(){
    for(auto &individuo : populacao){
        std::cout << "Caminho" << individuo.cromossomo() << "Fitness" << individuo.fitness << std::endl;
    }
}
