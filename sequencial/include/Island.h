#pragma once
#include <vector>
#include <cmath>
#include <random>
#include "Individuo.h"

struct Island {
    static int nextId;          // declaração apenas
    int idIlha;
    int seed;
    std::vector<int> vizinhos;
    std::vector<Individuo> populacao;
    Individuo melhorIndividuoIlhaa;
    int geracaoUltimaEvolucao = 0;
    bool reset = false;
    int resetNum = 200;
    std::mt19937 geradorlocal;
    Island(int seed); // construtor
    void imprimirPopulacao();
};
