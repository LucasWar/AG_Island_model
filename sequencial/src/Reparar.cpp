#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "utils.h"
#include "cvrpData.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <numeric>
#include <cmath>
#include <list>
#include <map>

std::vector<int> GeneticAlgorithm::repararCVRP(const std::vector<int>& clientes) {
    // --- FASE 1: CONSTRUÇÃO GULOSA (GARANTE CAPACIDADE) ---
    std::vector<std::vector<int>> rotas;
    std::vector<double> cargas;

    if (clientes.empty()) {
        return {0, 0};
    }
    
    // Inicia a primeira rota
    rotas.push_back({});
    cargas.push_back(0.0);

    for (int cliente : clientes) {
        double demanda = dataCVRP.demandas[cliente];

        // Tenta alocar na última rota criada
        if (cargas.back() + demanda <= dataCVRP.capacidade) {
            rotas.back().push_back(cliente);
            cargas.back() += demanda;
        } else {
            // Se não couber, cria uma nova rota para este cliente
            rotas.push_back({cliente});
            cargas.push_back(demanda);
        }
    }

    // --- FASE 2: REPARO (GARANTE NÚMERO DE VEÍCULOS) ---
    if (rotas.size() <= dataCVRP.numVeiculos) {
        // Solução já é viável, apenas formate a saída
        std::vector<int> solucaoFinal;
        solucaoFinal.push_back(0);
        for (const auto& rota : rotas) {
            solucaoFinal.insert(solucaoFinal.end(), rota.begin(), rota.end());
            solucaoFinal.push_back(0);
        }
        return solucaoFinal;
    }

    // Se excedeu o número de veículos, precisamos reparar
    std::vector<int> clientesOrfaos;
    
    // Coleta clientes das rotas extras
    while (rotas.size() > dataCVRP.numVeiculos) {
        clientesOrfaos.insert(clientesOrfaos.end(), rotas.back().begin(), rotas.back().end());
        rotas.pop_back();
        cargas.pop_back();
    }

    // Tenta reinserir os clientes órfãos nas rotas válidas
    bool todosInseridos = true;
    for (int orfao : clientesOrfaos) {
        double demandaOrfao = dataCVRP.demandas[orfao];
        bool inserido = false;

        // Procura a melhor posição de inserção (aqui, a primeira que couber)
        // Uma melhoria seria buscar a inserção que gerasse o menor custo adicional.
        for (int i = 0; i < rotas.size(); ++i) {
            if (cargas[i] + demandaOrfao <= dataCVRP.capacidade) {
                rotas[i].push_back(orfao); // Inserção simples no final
                cargas[i] += demandaOrfao;
                inserido = true;
                break; // Vai para o próximo órfão
            }
        }
        
        if (!inserido) {
            todosInseridos = false;
            break; // Se um órfão não puder ser inserido, a reparação falhou
        }
    }

    if (todosInseridos) {
        // Sucesso na reparação! Formate a saída.
        std::vector<int> solucaoFinal;
        solucaoFinal.push_back(0);
        for (const auto& rota : rotas) {
            solucaoFinal.insert(solucaoFinal.end(), rota.begin(), rota.end());
            solucaoFinal.push_back(0);
        }
        return solucaoFinal;
    } else {
        // Reparação impossível para esta permutação de clientes.
        // Retornar um vetor vazio sinaliza a falha.
        // O Algoritmo Genético deve então penalizar fortemente esta solução.
        return {}; 
    }
}