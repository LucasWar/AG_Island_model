#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "utils.h"
#include "cvrpData.h"
#include "Fitness.h"
#include <algorithm>
#include <unordered_set>
#include <numeric>
#include <random>
#include <cmath>
#include <map>

// =============================================================
// Funções utilitárias auxiliares
// =============================================================

static inline double calcularCarga(const std::vector<int>& rota, const CVRPData& data) {
    double carga = 0.0;
    for (int cliente : rota) carga += data.demandas[cliente];
    return carga;
}

static inline int escolherIndiceAleatorio(int limite, std::mt19937& gerador) {
    std::uniform_int_distribution<int> dist(0, limite - 1);
    return dist(gerador);
}

static inline int escolherRotaValida(const std::vector<std::vector<int>>& rotas, std::mt19937& gerador) {
    if (rotas.empty()) return -1;
    std::uniform_int_distribution<int> dist(0, rotas.size() - 1);
    return dist(gerador);
}

// =============================================================
// Tipos de mutação
// =============================================================

static void mutacaoSwap(std::vector<std::vector<int>>& rotas, std::mt19937& gerador) {
    int idx = escolherRotaValida(rotas, gerador);
    if (idx < 0 || rotas[idx].size() < 2) return;

    int p1 = escolherIndiceAleatorio(rotas[idx].size(), gerador);
    int p2 = (p1 + escolherIndiceAleatorio(rotas[idx].size() - 1, gerador) + 1) % rotas[idx].size();

    std::swap(rotas[idx][p1], rotas[idx][p2]);
}

static void mutacaoInversao(std::vector<std::vector<int>>& rotas, std::mt19937& gerador) {
    int idx = escolherRotaValida(rotas, gerador);
    if (idx < 0 || rotas[idx].size() < 2) return;

    int p1 = escolherIndiceAleatorio(rotas[idx].size(), gerador);
    int p2 = escolherIndiceAleatorio(rotas[idx].size(), gerador);
    if (p1 > p2) std::swap(p1, p2);

    std::reverse(rotas[idx].begin() + p1, rotas[idx].begin() + p2 + 1);
}

static void mutacaoMoverCliente(std::vector<std::vector<int>>& rotas, const CVRPData& data, std::mt19937& gerador) {
    if (rotas.size() < 2) return;

    int origem = escolherRotaValida(rotas, gerador);
    int destino = escolherRotaValida(rotas, gerador);
    if (origem < 0 || destino < 0 || origem == destino || rotas[origem].empty()) return;

    int posCliente = escolherIndiceAleatorio(rotas[origem].size(), gerador);
    int cliente = rotas[origem][posCliente];
    double cargaDestino = calcularCarga(rotas[destino], data);

    if (cargaDestino + data.demandas[cliente] <= data.capacidade) {
        rotas[origem].erase(rotas[origem].begin() + posCliente);
        int posInsercao = escolherIndiceAleatorio(rotas[destino].size() + 1, gerador);
        rotas[destino].insert(rotas[destino].begin() + posInsercao, cliente);
    }
}

static void mutacaoTrocarClientes(std::vector<std::vector<int>>& rotas, const CVRPData& data, std::mt19937& gerador) {
    if (rotas.size() < 2) return;

    int r1 = escolherRotaValida(rotas, gerador);
    int r2 = escolherRotaValida(rotas, gerador);
    if (r1 < 0 || r2 < 0 || r1 == r2 || rotas[r1].empty() || rotas[r2].empty()) return;

    int p1 = escolherIndiceAleatorio(rotas[r1].size(), gerador);
    int p2 = escolherIndiceAleatorio(rotas[r2].size(), gerador);

    int c1 = rotas[r1][p1];
    int c2 = rotas[r2][p2];

    double carga1 = calcularCarga(rotas[r1], data) - data.demandas[c1] + data.demandas[c2];
    double carga2 = calcularCarga(rotas[r2], data) - data.demandas[c2] + data.demandas[c1];

    if (carga1 <= data.capacidade && carga2 <= data.capacidade) {
        std::swap(rotas[r1][p1], rotas[r2][p2]);
    }
}

// =============================================================
// Reconstrução e aplicação
// =============================================================

static std::vector<int> reconstruirGenes(const std::vector<std::vector<int>>& rotas) {
    std::vector<int> genes = {0};
    for (const auto& rota : rotas) {
        if (!rota.empty()) {
            genes.insert(genes.end(), rota.begin(), rota.end());
            genes.push_back(0);
        }
    }
    if (genes.size() == 1) genes.push_back(0);
    return genes;
}

// =============================================================
// Mutação principal CVRP
// =============================================================

void GeneticAlgorithm::mutacaoCVRP(Individuo& ind, std::mt19937& geradorLocal) {
    if (ind.genes.size() <= 3) return;

    auto rotas = extrairRotas(ind.genes);
    if (rotas.empty()) return;

    int escolha = std::uniform_int_distribution<int>(1, 100)(geradorLocal);

    if (escolha <= 25) mutacaoSwap(rotas, geradorLocal);
    else if (escolha <= 50) mutacaoInversao(rotas, geradorLocal);
    else if (escolha <= 85) mutacaoMoverCliente(rotas, dataCVRP, geradorLocal);
    else mutacaoTrocarClientes(rotas, dataCVRP, geradorLocal);

    ind.genes = reconstruirGenes(rotas);
    ind.fitness = calcularFitness(ind.genes, dataCVRP);
}
