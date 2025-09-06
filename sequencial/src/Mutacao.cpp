#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "utils.h"
#include "cvrpData.h"
#include "Fitness.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <numeric>
#include <cmath>
#include <list>
#include <map> 

std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes) {
    std::vector<std::vector<int>> rotas;
    if (genes.empty()) return rotas;

    std::vector<int> rotaAtual;
    for (size_t i = 1; i < genes.size(); ++i) {
        if (genes[i] != 0) {
            rotaAtual.push_back(genes[i]);
        } else {
            if (!rotaAtual.empty()) {
                rotas.push_back(rotaAtual);
                rotaAtual.clear();
            }
        }
    }
    return rotas;
}

// void GeneticAlgorithm::mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal) {
//     if (ind.genes.size() <= 3) return; // Não há o que mutar

//     auto rotas = extrairRotas(ind.genes);
//     if (rotas.empty()) return;

//     std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
//     int idx_rota = dist_rotas(geradorLocal);
    
   
//     if (rotas[idx_rota].size() >= 2) {
//         std::uniform_int_distribution<int> dist_clientes(0, rotas[idx_rota].size() - 1);
//         int pos1 = dist_clientes(geradorLocal);
//         int pos2 = dist_clientes(geradorLocal);
//         while (pos1 == pos2) {
//             pos2 = dist_clientes(geradorLocal);
//         }
//         std::swap(rotas[idx_rota][pos1], rotas[idx_rota][pos2]);
//         //std::reverse(rotas[idx_rota].begin() + pos1, rotas[idx_rota].begin() + pos2);
//     }

//     std::vector<int> novos_genes;
//     novos_genes.push_back(0);
//     for (const auto& rota : rotas) {
//         for (int cliente : rota) {
//             novos_genes.push_back(cliente);
//         }
//         novos_genes.push_back(0);
//     }
    
//     ind.genes = novos_genes;
//     ind.fitness = calcularFitness(ind.genes,dataCVRP); // Recalcula o fitness com a função correta
// }



// Função para recalcular a carga de uma rota
double calcularCarga(const std::vector<int>& rota, const CVRPData& data) {
    double carga = 0.0;
    for (int cliente : rota) {
        carga += data.demandas[cliente];
    }
    return carga;
}

// OPERADOR 1: SWAP (O seu, levemente ajustado)
// Refina a ordem de uma única rota.
void mutacaoSwap(std::vector<std::vector<int>>& rotas, std::mt19937& gerador) {
    if (rotas.empty()) return;
    
    std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
    int idx_rota = dist_rotas(gerador);

    if (rotas[idx_rota].size() >= 2) {
        std::uniform_int_distribution<int> dist_clientes(0, rotas[idx_rota].size() - 1);
        int pos1 = dist_clientes(gerador);
        int pos2 = dist_clientes(gerador);
        if (pos1 != pos2) {
            std::swap(rotas[idx_rota][pos1], rotas[idx_rota][pos2]);
        }
    }
}

// OPERADOR 2: INVERSÃO (2-Opt)
// Desembaraça cruzamentos em uma rota.
void mutacaoInversao(std::vector<std::vector<int>>& rotas, std::mt19937& gerador) {
    if (rotas.empty()) return;

    std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
    int idx_rota = dist_rotas(gerador);

    if (rotas[idx_rota].size() >= 2) {
        std::uniform_int_distribution<int> dist_clientes(0, rotas[idx_rota].size() - 1);
        int pos1 = dist_clientes(gerador);
        int pos2 = dist_clientes(gerador);
        if (pos1 != pos2) {
            if (pos1 > pos2) std::swap(pos1, pos2);
            std::reverse(rotas[idx_rota].begin() + pos1, rotas[idx_rota].begin() + pos2 + 1);
        }
    }
}

// OPERADOR 3: MOVER CLIENTE (Relocate)
// Move um cliente de uma rota para outra. Fundamental para a exploração.
void mutacaoMoverCliente(std::vector<std::vector<int>>& rotas, const CVRPData& data, std::mt19937& gerador) {
    if (rotas.size() < 2) return; // Precisa de pelo menos duas rotas para a operação

    std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
    int idx_origem = dist_rotas(gerador);
    int idx_destino = dist_rotas(gerador);
    if (rotas[idx_origem].empty()) return; // Rota de origem não pode ser vazia
    while (idx_origem == idx_destino) {
        idx_destino = dist_rotas(gerador);
    }
    
    // Seleciona um cliente para mover
    std::uniform_int_distribution<int> dist_clientes_origem(0, rotas[idx_origem].size() - 1);
    int pos_cliente = dist_clientes_origem(gerador);
    int cliente = rotas[idx_origem][pos_cliente];

    // Verifica se a capacidade da rota de destino permite a inserção
    double carga_destino = calcularCarga(rotas[idx_destino], data);
    if (carga_destino + data.demandas[cliente] <= data.capacidade) {
        // Move o cliente
        rotas[idx_origem].erase(rotas[idx_origem].begin() + pos_cliente);
        
        // Insere em uma posição aleatória na rota de destino
        std::uniform_int_distribution<int> dist_pos_destino(0, rotas[idx_destino].size());
        rotas[idx_destino].insert(rotas[idx_destino].begin() + dist_pos_destino(gerador), cliente);
    }
}

// OPERADOR 4: TROCAR CLIENTES (Exchange)
// Troca um cliente de uma rota com um cliente de outra rota.
void mutacaoTrocarClientes(std::vector<std::vector<int>>& rotas, const CVRPData& data, std::mt19937& gerador) {
    if (rotas.size() < 2) return;

    std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
    int idx1 = dist_rotas(gerador);
    int idx2 = dist_rotas(gerador);
    if (rotas[idx1].empty() || rotas[idx2].empty()) return;
    while (idx1 == idx2) {
        idx2 = dist_rotas(gerador);
        if(rotas[idx2].empty()) return;
    }

    std::uniform_int_distribution<int> dist_c1(0, rotas[idx1].size() - 1);
    std::uniform_int_distribution<int> dist_c2(0, rotas[idx2].size() - 1);
    int pos1 = dist_c1(gerador);
    int pos2 = dist_c2(gerador);
    
    int cliente1 = rotas[idx1][pos1];
    int cliente2 = rotas[idx2][pos2];

    // Verifica se a troca mantém as capacidades válidas
    double carga1_sem_c1 = calcularCarga(rotas[idx1], data) - data.demandas[cliente1];
    double carga2_sem_c2 = calcularCarga(rotas[idx2], data) - data.demandas[cliente2];

    if (carga1_sem_c1 + data.demandas[cliente2] <= data.capacidade &&
        carga2_sem_c2 + data.demandas[cliente1] <= data.capacidade) {
        // Realiza a troca
        rotas[idx1][pos1] = cliente2;
        rotas[idx2][pos2] = cliente1;
    }
}




void GeneticAlgorithm::mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal) {
    if (ind.genes.size() <= 3) return;

    auto rotas = extrairRotas(ind.genes);
    if (rotas.empty()) return;

    // Escolhe qual operador de mutação usar com base em probabilidades
    std::uniform_int_distribution<int> dist_operador(1, 100);
    int escolha = dist_operador(geradorLocal);

    if (escolha <= 25) { // 25% de chance para Swap
        mutacaoSwap(rotas, geradorLocal);
    } else if (escolha <= 50) { // 25% de chance para Inversão
        mutacaoInversao(rotas, geradorLocal);
    } else if (escolha <= 85) { // 35% de chance para Mover Cliente
        mutacaoMoverCliente(rotas, dataCVRP, geradorLocal);
    } else { // 15% de chance para Trocar Clientes
        mutacaoTrocarClientes(rotas, dataCVRP, geradorLocal);
    }

    // --- Reconstrução dos genes (mesma lógica de antes) ---
    std::vector<int> novos_genes;
    novos_genes.push_back(0);
    for (auto& rota : rotas) {
        // Remove rotas que podem ter ficado vazias após a mutação
        if (!rota.empty()) {
            novos_genes.insert(novos_genes.end(), rota.begin(), rota.end());
            novos_genes.push_back(0);
        }
    }
    
    // Garante que a representação seja válida mesmo que não haja clientes
    if (novos_genes.size() == 1 && novos_genes[0] == 0) {
        novos_genes.push_back(0);
    }

    ind.genes = novos_genes;
    ind.fitness = calcularFitness(ind.genes, dataCVRP);
}