#include "Crossover.h"
#include "TypesUtils.h"
#include "Fitness.h"
#include "Reparar.h"
#include <algorithm>
#include <random>

// Implementação do OX
Individuo OXCrossover::aplicar(const Individuo &pai1,
                                    const Individuo &pai2,
                                    std::mt19937 &geradorLocal,
                                    const CVRPData &dataCVRP) {

    // --- Extrair só os clientes (ignorar zeros) ---
    std::vector<int> clientes1, clientes2;
    for (int g : pai1.genes) if (g != 0) clientes1.push_back(g);
    for (int g : pai2.genes) if (g != 0) clientes2.push_back(g);

    int n = clientes1.size();
    std::uniform_int_distribution<int> dist(0, n - 1);
    int cut1 = dist(geradorLocal);
    int cut2 = dist(geradorLocal);
    if (cut1 > cut2) std::swap(cut1, cut2);

    std::vector<int> filho(n, -1);

    // Copiar segmento do pai1
    for (int i = cut1; i <= cut2; i++) {
        filho[i] = clientes1[i];
    }

    // Preencher com clientes do pai2 na ordem
    int pos = (cut2 + 1) % n;
    for (int c : clientes2) {
        if (std::find(filho.begin(), filho.end(), c) == filho.end()) {
            filho[pos] = c;
            pos = (pos + 1) % n;
        }
    }

    // --- Neste ponto "filho" é uma permutação de clientes, SEM rotas ---
    // Agora precisamos reconstruir rotas respeitando a capacidade
    std::vector<int> genesCorrigidos = repararCVRP(filho,dataCVRP);
    if(genesCorrigidos.empty()){
        return pai1;
    }
    return Individuo(genesCorrigidos, calcularFitness(genesCorrigidos,dataCVRP));
    return {}; 
}

// Implementação do PMX
Individuo PMXCrossover::aplicar(const Individuo &pai1,
                                       const Individuo &pai2,
                                       std::mt19937 &geradorLocal,
                                       const CVRPData &dataCVRP) {
   // 1. Extrair apenas os clientes (sem os 0s)
    std::vector<int> clientes1, clientes2;
    for (int g : pai1.genes) if (g != 0) clientes1.push_back(g);
    for (int g : pai2.genes) if (g != 0) clientes2.push_back(g);

    int n = clientes1.size();
    std::vector<int> filho(n, -1);

    // 2. Sorteia dois pontos de corte
    std::uniform_int_distribution<int> dist(0, n - 1);
    int c1 = dist(geradorLocal);
    int c2 = dist(geradorLocal);
    if (c1 > c2) std::swap(c1, c2);

    // 3. Copiar segmento do pai1 para o filho
    for (int i = c1; i <= c2; i++) {
        filho[i] = clientes1[i];
    }

    // 4. Mapear genes do pai2 → preencher conflitos
    for (int i = c1; i <= c2; i++) {
        int gene = clientes2[i];
        if (std::find(filho.begin(), filho.end(), gene) == filho.end()) {
            int pos = i;
            while (filho[pos] != -1) {
                int gene_p1 = clientes1[pos];
                pos = std::find(clientes2.begin(), clientes2.end(), gene_p1) - clientes2.begin();
            }
            filho[pos] = gene;
        }
    }

    // 5. Preencher os espaços vazios restantes com genes do pai2
    for (int i = 0; i < n; i++) {
        if (filho[i] == -1) {
            filho[i] = clientes2[i];
        }
    }

    // 6. Reconstruir cromossomo válido via reparador
    std::vector<int> cromossomo = repararCVRP(filho,dataCVRP); 
    if(cromossomo.empty()){
        return pai1;
    }
    Individuo novo;
    novo.genes = cromossomo;
    novo.fitness = calcularFitness(cromossomo,dataCVRP);

    return novo;

}
