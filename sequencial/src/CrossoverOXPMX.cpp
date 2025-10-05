#include "Crossover.h"
#include "TypesUtils.h"
#include "Fitness.h"
#include "Reparar.h"
#include <algorithm>
#include <random>
#include <unordered_set>
#include <unordered_map>

// ======================
// OX Crossover (otimizado)
// ======================
Individuo OXCrossover::aplicar(const Individuo &pai1,
                               const Individuo &pai2,
                               std::mt19937 &geradorLocal,
                               const CVRPData &dataCVRP,
                               bool aplicar2opt) {

    // Extrair clientes (sem zeros)
    std::vector<int> clientes1, clientes2;
    clientes1.reserve(pai1.genes.size());
    clientes2.reserve(pai2.genes.size());

    for (int g : pai1.genes) if (g != 0) clientes1.push_back(g);
    for (int g : pai2.genes) if (g != 0) clientes2.push_back(g);

    int n = clientes1.size();
    if (n == 0) return pai1;

    // Selecionar pontos de corte
    std::uniform_int_distribution<int> dist(0, n - 1);
    int cut1 = dist(geradorLocal);
    int cut2 = dist(geradorLocal);
    if (cut1 > cut2) std::swap(cut1, cut2);

    std::vector<int> filho(n, -1);
    std::unordered_set<int> usados;

    // Copiar segmento do pai1
    for (int i = cut1; i <= cut2; i++) {
        filho[i] = clientes1[i];
        usados.insert(clientes1[i]);
    }

    // Preencher com clientes do pai2 na ordem
    int pos = (cut2 + 1) % n;
    for (int c : clientes2) {
        if (!usados.count(c)) {
            filho[pos] = c;
            usados.insert(c);
            pos = (pos + 1) % n;
        }
    }

    // Reparar e avaliar fitness
    std::vector<int> genesCorrigidos = repararCVRP(filho, dataCVRP,aplicar2opt);
    if (genesCorrigidos.empty()) return pai1;

    return Individuo(genesCorrigidos, calcularFitness(genesCorrigidos, dataCVRP));
}


// ======================
// PMX Crossover (otimizado)
// ======================
Individuo PMXCrossover::aplicar(const Individuo &pai1,
                                const Individuo &pai2,
                                std::mt19937 &geradorLocal,
                                const CVRPData &dataCVRP,
                                bool aplicar2opt) {

    // Extrair clientes (sem zeros)
    std::vector<int> clientes1, clientes2;
    clientes1.reserve(pai1.genes.size());
    clientes2.reserve(pai2.genes.size());

    for (int g : pai1.genes) if (g != 0) clientes1.push_back(g);
    for (int g : pai2.genes) if (g != 0) clientes2.push_back(g);

    int n = clientes1.size();
    if (n == 0) return pai1;

    std::vector<int> filho(n, -1);

    // Criar mapa de posição para o pai2 (lookup O(1))
    std::unordered_map<int, int> posPai2;
    posPai2.reserve(n);
    for (int i = 0; i < n; i++) {
        posPai2[clientes2[i]] = i;
    }

    // Selecionar pontos de corte
    std::uniform_int_distribution<int> dist(0, n - 1);
    int c1 = dist(geradorLocal);
    int c2 = dist(geradorLocal);
    if (c1 > c2) std::swap(c1, c2);

    // Copiar segmento do pai1 para o filho
    for (int i = c1; i <= c2; i++) {
        filho[i] = clientes1[i];
    }

    // Resolver conflitos com mapeamento
    for (int i = c1; i <= c2; i++) {
        int gene = clientes2[i];
        if (std::find(filho.begin() + c1, filho.begin() + c2 + 1, gene) == filho.begin() + c2 + 1) {
            int pos = i;
            while (filho[pos] != -1) {
                int gene_p1 = clientes1[pos];
                pos = posPai2[gene_p1];
            }
            filho[pos] = gene;
        }
    }

    // Preencher os espaços vazios restantes com genes do pai2
    for (int i = 0; i < n; i++) {
        if (filho[i] == -1) {
            filho[i] = clientes2[i];
        }
    }

    // Reparar e avaliar fitness
    std::vector<int> cromossomo = repararCVRP(filho, dataCVRP,aplicar2opt);
    if (cromossomo.empty()) return pai1;

    return Individuo(cromossomo, calcularFitness(cromossomo, dataCVRP));
}


Individuo RBXCrossover::aplicar(const Individuo& pai1, const Individuo& pai2,std::mt19937& gerador, const CVRPData& data, bool aplicar2opt){
        auto extrairRotas = [](const std::vector<int>& genes) {
            std::vector<std::vector<int>> rotas;
            std::vector<int> atual;
            for (int g : genes) {
                if (g == 0) {
                    if (!atual.empty()) {
                        rotas.push_back(atual);
                        atual.clear();
                    }
                } else {
                    atual.push_back(g);
                }
            }
            return rotas;
        };

        auto rotas1 = extrairRotas(pai1.genes);
        auto rotas2 = extrairRotas(pai2.genes);

        // Seleciona aleatoriamente rotas de P1
        std::unordered_set<int> clientesSelecionados;
        std::vector<std::vector<int>> rotasFilho;

        std::uniform_int_distribution<int> distR(1, std::max(1, static_cast<int>(rotas1.size() / 2)));
        int numRotasSelecionar = distR(gerador);

        std::vector<int> indices(rotas1.size());
        std::iota(indices.begin(), indices.end(), 0);
        std::shuffle(indices.begin(), indices.end(), gerador);

        for (int i = 0; i < numRotasSelecionar; ++i) {
            auto& rota = rotas1[indices[i]];
            rotasFilho.push_back(rota);
            for (int c : rota) {
                clientesSelecionados.insert(c);
            }
        }

        // Adiciona clientes restantes com base na ordem de P2
        std::vector<int> resto;
        for (const auto& rota : rotas2) {
            for (int c : rota) {
                if (clientesSelecionados.find(c) == clientesSelecionados.end()) {
                    resto.push_back(c);
                    clientesSelecionados.insert(c);
                }
            }
        }

        // Junta todos para gerar o filho
        std::vector<int> filhoGenes;
        for (const auto& rota : rotasFilho) {
            filhoGenes.insert(filhoGenes.end(), rota.begin(), rota.end());
        }
        filhoGenes.insert(filhoGenes.end(), resto.begin(), resto.end());

        // Repara para gerar solução viável
        std::vector<int> filhoReparado = repararCVRP(filhoGenes, data,aplicar2opt);
        if (filhoReparado.empty()) {
            // Se reparo falhar, devolve pai1 (ou penaliza)
            return pai1;
        }

        return Individuo(filhoReparado, calcularFitness(filhoReparado, data));
}
