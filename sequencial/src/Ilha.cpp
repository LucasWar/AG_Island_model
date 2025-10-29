#include "Ilha.h"
#include <iostream>
#include <random>

// definição do static
int Ilha::nextId = 0;

// construtor
Ilha::Ilha(int seed) : idIlha(nextId++), seed(seed), geradorlocal(idIlha + seed){} 

void Ilha::imprimirPopulacao(){
    for(auto &individuo : populacao){
        std::cout << "Caminho: " << individuo.cromossomo() << " Fitness: " << individuo.fitness << std::endl;
    }
}
