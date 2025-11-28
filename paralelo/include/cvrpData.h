#pragma once
#include <vector>
#include <string> 

struct CVRPData {
    std::vector<std::vector<double>> distancias; // matriz NxN
    std::vector<int> demandas;                   // vetor de demandas
    std::string nome;
    int capacidade;                              // capacidade dos veículos
    int deposito;                                // nó do depósito
    int numVeiculos;
    int solucaoOtima;
};