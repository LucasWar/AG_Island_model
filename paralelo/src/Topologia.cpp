#include "Topologia.h"
#include "Individuo.h"
#include "cvrpData.h"
#include <iostream>
#include "Ilha.h"
#include <vector>
#include <string>
#include <random>

std::vector<Ilha> Topologia::criarTopologia(std::string opcTopologia, int numIlhas, int seed, std::string crossover, std::string selecao) {
    Ilha::nextId = 0; // Reseta o contador de IDs estático

    // =========================================================
    // CASO 1: MODO SEQUENCIAL (1 ILHA APENAS)
    // Evita chamar criarMalha/Anel desnecessariamente
    // =========================================================
    if (numIlhas == 1) {
        // Validação básica de entrada
        if(selecao == "None" || crossover == "None"){
            return {};
        }

        std::vector<Ilha> ilhas;
        ilhas.reserve(1);
        
        // Cria a ilha diretamente (equivalente ao que criarMalha faria, mas sem vizinhos)
        ilhas.emplace_back(seed); 
        
        Ilha* ilha = &ilhas[0];

        // Configuração Padrão da Ilha Sequencial (Geralmente Híbrida/Equilibrada)
        ilha->tipoSelecao = selecao;
        ilha->tamanhoTorneio = 3;     // Pressão média
        ilha->proMutacao = 0.25;      // Taxa padrão
        ilha->usaBuscaLocal = true;   // 2-Opt ativado

        // Seleção do Crossover baseada no parâmetro
        if (crossover == "PMX") {
            ilha->crossoverilha = std::make_unique<PMXCrossover>();
        } else if (crossover == "OX") {
            ilha->crossoverilha = std::make_unique<OXCrossover>();
        } else {
            ilha->crossoverilha = std::make_unique<RBXCrossover>();
        }

        std::cout << "Modo: 1 Ilha (Sequencial)" << std::endl;
        std::cout << "Crossover selecionado: " << crossover << std::endl;
        return ilhas;
    }

    // =========================================================
    // CASO 2: MODO PARALELO (MÚLTIPLAS ILHAS)
    // Cria a topologia e aplica a configuração heterogênea
    // =========================================================
    std::vector<Ilha> ilhas = {};
    
    if (opcTopologia == "Malha") {
        ilhas = criarMalha(numIlhas, seed);
    } else if (opcTopologia == "Anel") {
        ilhas = criarAnel(numIlhas, seed);
    } 

    if(ilhas.empty()) {
        return {};
    }

    std::cout << "Numero de ilhas: " << numIlhas << std::endl;
    std::cout << "Topologia Selecionada: "<< opcTopologia << std::endl;

    // Configuração das Estratégias Heterogêneas
    for (auto &ilha : ilhas) {
        if (ilha.idIlha % 3 == 0) {
            // ILHA TIPO 0: Intensificação (PMX, Torneio Forte, Mutação Baixa)
            ilha.tipoSelecao = "Torneio";
            ilha.tamanhoTorneio = 5;
            ilha.proMutacao = 0.15;
            ilha.crossoverilha = std::make_unique<PMXCrossover>();
            ilha.usaBuscaLocal = true; 

        } else if (ilha.idIlha % 3 == 1) {
            // ILHA TIPO 1: Exploração via Mutação (A versão corrigida que discutimos)
            ilha.tipoSelecao = "Torneio"; 
            ilha.tamanhoTorneio = 2;     // Baixa pressão
            ilha.proMutacao = 0.50;      // Alta mutação (Exploração)
            ilha.crossoverilha = std::make_unique<PMXCrossover>(); // PMX (melhor que OX)
            ilha.usaBuscaLocal = true;   // Com 2-Opt

        } else {
            // ILHA TIPO 2: Híbrida (RBX, Torneio Médio, Mutação Média)
            ilha.tipoSelecao = "Torneio";
            ilha.tamanhoTorneio = 3;
            ilha.proMutacao = 0.25;
            ilha.crossoverilha = std::make_unique<RBXCrossover>();
            ilha.usaBuscaLocal = true;
        }
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