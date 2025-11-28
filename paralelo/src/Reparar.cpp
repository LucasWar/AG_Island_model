#include "algoritmoGenetico.h"
#include "cvrpData.h"
#include "Utils.h"
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

void aplicar2Opt(std::vector<int>& rota, const std::vector<std::vector<double>>& distancias) {
    bool melhorou = true;
    while (melhorou) {
        melhorou = false;
        for (size_t i = 1; i + 1 < rota.size(); ++i) {
            for (size_t j = i + 1; j < rota.size(); ++j) {
                int anterior = rota[i - 1];
                int atual = rota[i];
                int proximo = (j + 1 < rota.size()) ? rota[j + 1] : 0;

                double custoAtual = distancias[anterior][atual] + distancias[rota[j]][proximo];
                double custoNovo = distancias[anterior][rota[j]] + distancias[atual][proximo];

                if (custoNovo < custoAtual - 1e-6) {
                    std::reverse(rota.begin() + i, rota.begin() + j + 1);
                    melhorou = true;
                }
            }
        }
    }
}

// ----------------------------
// Reparador CVRP otimizado
// ----------------------------
std::vector<int> repararCVRP(const std::vector<int>& clientes, CVRPData dataCVRP, bool aplicar2opt) {
    std::vector<std::vector<int>> rotas;
    std::vector<double> cargas;

    if (clientes.empty()) {
        return {0, 0};
    }

    if(verificarValidadeCVRP(clientes,dataCVRP)){
        return clientes;
    }

    // ----------------------------
    // FASE 1: Construção GULOSA simples por capacidade
    // ----------------------------
    rotas.push_back({});
    cargas.push_back(0.0);

    for (int cliente : clientes) {
        double demanda = dataCVRP.demandas[cliente];

        if (cargas.back() + demanda <= dataCVRP.capacidade) {
            rotas.back().push_back(cliente);
            cargas.back() += demanda;
        } else {
            rotas.push_back({cliente});
            cargas.push_back(demanda);
        }
    }

    // ----------------------------
    // FASE 2: Reparo (garante número de veículos)
    // ----------------------------
    std::vector<int> clientesOrfaos;

    while (rotas.size() > dataCVRP.numVeiculos) {
        clientesOrfaos.insert(clientesOrfaos.end(), rotas.back().begin(), rotas.back().end());
        rotas.pop_back();
        cargas.pop_back();
    }

    for (int orfao : clientesOrfaos) {
        double demanda = dataCVRP.demandas[orfao];
        double melhorCusto = std::numeric_limits<double>::max();
        int melhorRota = -1;
        int melhorPos = -1;

        // Procurar a melhor inserção com menor custo adicional
        for (int i = 0; i < rotas.size(); ++i) {
            if (cargas[i] + demanda > dataCVRP.capacidade) continue;

            auto& rota = rotas[i];
            for (size_t pos = 0; pos <= rota.size(); ++pos) {
                int anterior = (pos == 0) ? 0 : rota[pos - 1];
                int proximo = (pos == rota.size()) ? 0 : rota[pos];

                double custoAdicional =
                    dataCVRP.distancias[anterior][orfao] +
                    dataCVRP.distancias[orfao][proximo] -
                    dataCVRP.distancias[anterior][proximo];

                if (custoAdicional < melhorCusto) {
                    melhorCusto = custoAdicional;
                    melhorRota = i;
                    melhorPos = pos;
                }
            }
        }

        if (melhorRota == -1) {
            // Não foi possível inserir este cliente
            return {};
        }

        // Inserir na melhor posição da melhor rota
        rotas[melhorRota].insert(rotas[melhorRota].begin() + melhorPos, orfao);
        cargas[melhorRota] += demanda;
    }

    // ----------------------------
    // FASE 3: Aplicar 2-opt em cada rota
    // ----------------------------
    if(aplicar2opt){
        for (auto& rota : rotas) {
            aplicar2Opt(rota, dataCVRP.distancias);
        }
    }
    // ----------------------------
    // FASE 4: Formatar saída com separadores (0)
    // ----------------------------
    std::vector<int> solucaoFinal;
    solucaoFinal.push_back(0);
    for (const auto& rota : rotas) {
        solucaoFinal.insert(solucaoFinal.end(), rota.begin(), rota.end());
        solucaoFinal.push_back(0);
    }

    return solucaoFinal;
}