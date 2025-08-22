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

void GeneticAlgorithm::mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal) {
    if (ind.genes.size() <= 3) return; // Não há o que mutar

    auto rotas = extrairRotas(ind.genes);
    if (rotas.empty()) return;

    std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
    int idx_rota = dist_rotas(geradorLocal);
    
   
    if (rotas[idx_rota].size() >= 2) {
        std::uniform_int_distribution<int> dist_clientes(0, rotas[idx_rota].size() - 1);
        int pos1 = dist_clientes(geradorLocal);
        int pos2 = dist_clientes(geradorLocal);
        while (pos1 == pos2) {
            pos2 = dist_clientes(geradorLocal);
        }
        std::swap(rotas[idx_rota][pos1], rotas[idx_rota][pos2]);
        //std::reverse(rotas[idx_rota].begin() + pos1, rotas[idx_rota].begin() + pos2);
    }

    std::vector<int> novos_genes;
    novos_genes.push_back(0);
    for (const auto& rota : rotas) {
        for (int cliente : rota) {
            novos_genes.push_back(cliente);
        }
        novos_genes.push_back(0);
    }
    
    ind.genes = novos_genes;
    ind.fitness = calcularFitness(ind.genes); // Recalcula o fitness com a função correta
}