#include "Topologia.h"
#include "Individuo.h"
#include "cvrpData.h"
#include <iostream>
#include "Island.h"
#include <vector>
#include <string>
#include <random>

std::vector<Island> Topologia::criarTopologia(std::string opcTopologia, int numIlhas, int seed, std::string crossover,std::string selecao) {
    std::vector<Island> islands = {};
    if (opcTopologia == "Malha") {
        islands = criarMalha(numIlhas, seed);
    } else if (opcTopologia == "Anel") {
        islands = criarAnel(numIlhas, seed);
    } 

    if(islands.size() == 0) {
        return {};
    }
    std::cout << "Numero de ilhas: " << numIlhas << std::endl;
    std::cout << "Topologia Selecionada: "<< opcTopologia << std::endl;

    std::uniform_real_distribution<double> distLocal(0, 1);
    std::uniform_real_distribution<double> probMutGerador(0.2, 0.6);
    if(islands.size() > 1){
        for (auto &island : islands) {
            island.proMutacao = probMutGerador(island.geradorlocal);
            // Alternância forçada entre OX e PMX para diversidade
            if (island.idIlha % 3 == 0) {
                island.tipoSelecao = "Torneio";
                island.proMutacao = 0.15;
                island.crossoverisland = std::make_unique<PMXCrossover>();
                island.usaBuscaLocal = true;
            } else if (island.idIlha % 3 == 1) {
                island.tipoSelecao = "Roleta";
                island.proMutacao = 0.05;
                island.crossoverisland = std::make_unique<OXCrossover>();
                island.usaBuscaLocal = false;
            } else {
                island.tipoSelecao = "Elitista";
                island.proMutacao = 0.25;
                island.crossoverisland = std::make_unique<RBXCrossover>();
                island.usaBuscaLocal = true;
            }
        }    
    }
    else{
        if(selecao == "None" or crossover == "None"){
            return {};
        }
        auto island = &islands[0];
        island->tipoSelecao = selecao;
        if (crossover == "PMX") {
            island->crossoverisland = std::make_unique<PMXCrossover>();
        } else if (crossover == "OX") {
            island->crossoverisland = std::make_unique<OXCrossover>();
        } else {
            island->crossoverisland = std::make_unique<RBXCrossover>();
        }
        island->proMutacao = 0.25;
        island->usaBuscaLocal = true;
        std::cout << "Crossover selecionado: " << crossover << std::endl;
    }

    
    

    return islands;
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

        if (linha > 0) ilha.vizinhos.push_back((linha - 1) * numCols + coluna);

        if ((linha + 1) * numCols + coluna < numIlhas) 
            ilha.vizinhos.push_back((linha + 1) * numCols + coluna);
       
        if (coluna > 0) ilha.vizinhos.push_back(linha * numCols + (coluna - 1));
        
        if (coluna < numCols - 1 && id + 1 < numIlhas) 
            ilha.vizinhos.push_back(linha * numCols + (coluna + 1));

        ilhas.push_back(std::move(ilha));
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
            ilha.vizinhos.push_back(1);
        }
    
        ilhas.push_back(std::move(ilha));
    }
    Island::nextId = 1;

    return ilhas;
}