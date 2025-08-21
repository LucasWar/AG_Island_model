#include "Topologia.h"
#include <vector>
#include <string>
#include "Individuo.h"
#include <random>
#include "Island.h"
#include "cvrpData.h"

std::vector<Island> Topologia::criarTopologia(TipoTopologia tipo, int numIlhas, int seed) {
    switch (tipo) {
        case TipoTopologia::Malha:
            return criarMalha(numIlhas, seed);
        case TipoTopologia::Anel:
            return criarAnel(numIlhas, seed);
        default:
            return {};
    }
}



std::vector<Island> Topologia::criarMalha(int numIlhas, int seed) {
    std::vector<Island> ilhas;
    int numLin = std::floor(std::sqrt(numIlhas));
    int numCols = std::ceil((double)numIlhas / numLin);

    ilhas.reserve(numIlhas);


    for (int id = 0; id < numIlhas; id++) {
        Island ilha(seed);
        

        int linha = ilha.idIlha / numCols;
        int coluna = ilha.idIlha % numCols;

        // cima
        if (linha > 0) ilha.vizinhos.push_back((linha - 1) * numCols + coluna);
        // baixo
        if ((linha + 1) * numCols + coluna < numIlhas) 
            ilha.vizinhos.push_back((linha + 1) * numCols + coluna);
        // esquerda
        if (coluna > 0) ilha.vizinhos.push_back(linha * numCols + (coluna - 1));
        // direita
        if (coluna < numCols - 1 && id + 1 < numIlhas) 
            ilha.vizinhos.push_back(linha * numCols + (coluna + 1));

        ilhas.push_back(ilha);
    }
    Island::nextId = 1;
    return ilhas;
}

std::vector<Island> Topologia::criarAnel(int numIlhas, int seed) {
    std::vector<Island> ilhas;
    for (int id = 0; id < numIlhas; id++) {
        Island ilha(seed);
        if(ilha.idIlha != numIlhas){
            ilha.vizinhos.push_back(ilha.idIlha + 1);
        }else{
            ilha.vizinhos.push_back(0);
        }
    
        ilhas.push_back(ilha);
    }
    Island::nextId = 1;

    return ilhas;
}