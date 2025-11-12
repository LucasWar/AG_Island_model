#pragma once
#include <vector>
#include <cmath>
#include <random>
#include "Individuo.h"
#include <memory>
#include "Crossover.h"

struct Ilha {
    static int nextId;          // declaração apenas
    int idIlha;
    int seed;
    int genMigracao;
    bool usaBuscaLocal = false;

    std::string tipoSelecao;

    float proMutacao;
    std::unique_ptr<ICrossover> crossoverilha; 
    
    int tamanhoTorneio = 2;
    std::vector<int> vizinhos;
    std::vector<Individuo> populacao;
    Individuo melhorIndividuoIlhaa;
    int geracaoUltimaEvolucao = 0;
    bool reset = false;
    int resetNum = 200;
    std::mt19937 geradorlocal;
    std::vector<std::mt19937> geradorlocalThread;
    Ilha(int seed); // construtor
    void imprimirPopulacao();
};
