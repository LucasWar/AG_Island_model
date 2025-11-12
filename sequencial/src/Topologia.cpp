#include "Topologia.h"
#include "Individuo.h"
#include "cvrpData.h"
#include <iostream>
#include "Ilha.h"
#include <vector>
#include <string>
#include <random>

std::vector<Ilha> Topologia::criarTopologia(std::string opcTopologia, int numIlhas, int seed, std::string crossover,std::string selecao) {
    Ilha::nextId = 0;
    std::vector<Ilha> ilhas = {};
    if (opcTopologia == "Malha") {
        ilhas = criarMalha(numIlhas, seed);
    } else if (opcTopologia == "Anel") {
        ilhas = criarAnel(numIlhas, seed);
    } 

    if(ilhas.size() == 0) {
        return {};
    }
    std::cout << "Numero de ilhas: " << numIlhas << std::endl;
    std::cout << "Topologia Selecionada: "<< opcTopologia << std::endl;

    std::uniform_real_distribution<double> distLocal(0, 1);
    std::uniform_real_distribution<double> probMutGerador(0.2, 0.6);
    if(ilhas.size() > 1){
        for (auto &ilha : ilhas) {
            if (ilha.idIlha % 3 == 0) {
                // ILHA 0: Intensificadora (Alta Pressão + 2-Opt)
                ilha.tipoSelecao = "Torneio";
                ilha.tamanhoTorneio = 5; // Alta pressão de seleção
                ilha.proMutacao = 0.15;
                ilha.crossoverilha = std::make_unique<PMXCrossover>();
                ilha.usaBuscaLocal = true; // 2-Opt ativado

            } else if (ilha.idIlha % 3 == 1) {
                // ILHA 1: Exploradora (Baixa Pressão + Sem 2-Opt)
                ilha.tipoSelecao = "Roleta"; // Baixa pressão de seleção
                // ilha.tamanhoTorneio = 2; (não usado pela roleta)
                ilha.proMutacao = 0.05;
                ilha.crossoverilha = std::make_unique<OXCrossover>();
                ilha.usaBuscaLocal = false; // 2-Opt desativado (explora mais)

            } else {
                // ILHA 2: Híbrida (Pressão Média + 2-Opt)
                ilha.tipoSelecao = "Torneio";
                ilha.tamanhoTorneio = 3; // Pressão de seleção média
                ilha.proMutacao = 0.25; // Taxa de mutação mais alta
                ilha.crossoverilha = std::make_unique<RBXCrossover>();
                ilha.usaBuscaLocal = true; // 2-Opt ativado
            }
        }    
    }
    else{
        if(selecao == "None" or crossover == "None"){
            return {};
        }
        auto ilha = &ilhas[0];
        ilha->tipoSelecao = selecao;
        if (crossover == "PMX") {
            ilha->crossoverilha = std::make_unique<PMXCrossover>();
        } else if (crossover == "OX") {
            ilha->crossoverilha = std::make_unique<OXCrossover>();
        } else {
            ilha->crossoverilha = std::make_unique<RBXCrossover>();
        }
        ilha->proMutacao = 0.25;
        ilha->usaBuscaLocal = true;
        std::cout << "Crossover selecionado: " << crossover << std::endl;
    }

    return ilhas;
}



std::vector<Ilha> Topologia::criarMalha(int numIlhas, int seed) {
    std::vector<Ilha> ilhas;
    int numLin = std::floor(std::sqrt(numIlhas));
    int numCols = std::ceil((double)numIlhas / numLin);

    ilhas.reserve(numIlhas);


    for (int id = 0; id < numIlhas; id++) {
        Ilha ilha(seed);
        

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
    return ilhas;
}

std::vector<Ilha> Topologia::criarAnel(int numIlhas, int seed) {
    std::vector<Ilha> ilhas;
    for (int id = 0; id < numIlhas; id++) {
        Ilha ilha(seed);
        if(ilha.idIlha != numIlhas - 1){
            ilha.vizinhos.push_back(ilha.idIlha + 1);
        }else{
            ilha.vizinhos.push_back(0);
        }
    
        ilhas.push_back(std::move(ilha));
    }

    return ilhas;
}