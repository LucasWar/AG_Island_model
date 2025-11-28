#include "algoritmoGenetico.h"
#include "cvrpData.h"
#include "Topologia.h"
#include "utils.h"
#include <algorithm>
#include <unordered_set>
#include <list>

std::vector<std::vector<int>> GeneticAlgorithm::extrairRotas(const std::vector<int>& genes) {
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

Individuo GeneticAlgorithm::crossoverOX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal){
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
    std::vector<int> genesCorrigidos = repararCVRP(filho);
    if(genesCorrigidos.empty()){
        return pai1;
    }
    return Individuo(genesCorrigidos, calcularFitness(genesCorrigidos));
}


Individuo GeneticAlgorithm::crossoverPMX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal){
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
    std::vector<int> cromossomo = repararCVRP(filho); 

    Individuo novo;
    novo.genes = cromossomo;
    novo.fitness = calcularFitness(cromossomo);

    return novo;
}

Individuo GeneticAlgorithm::crossoverCVRP(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal) {
    auto rotas_pai1 = extrairRotas(pai1.genes);
    auto rotas_pai2 = extrairRotas(pai2.genes);

    std::uniform_int_distribution<int> dist1(0, rotas_pai1.size() - 1);
    std::uniform_int_distribution<int> dist2(0, rotas_pai2.size() - 1);

    // Pega uma rota aleatória do pai 1
    std::vector<int> rota_herdada = rotas_pai1[dist1(geradorLocal)];
    
    std::unordered_set<int> clientes_servidos(rota_herdada.begin(), rota_herdada.end());
    
    // Constrói o filho
    Individuo filho;
    filho.genes.push_back(0);
    for(int cliente : rota_herdada) {
        filho.genes.push_back(cliente);
    }
    filho.genes.push_back(0);

    // Adiciona os clientes restantes do pai 2 que ainda não foram servidos
    std::vector<int> clientes_restantes;
    for (const auto& rota : rotas_pai2) {
        for (int cliente : rota) {
            if (clientes_servidos.find(cliente) == clientes_servidos.end()) {
                clientes_restantes.push_back(cliente);
            }
        }
    }
    
    // Usa a mesma lógica da geração de população para alocar os clientes restantes
    std::list<int> clientesNaoAtendidos(clientes_restantes.begin(), clientes_restantes.end());
    int veiculosUsados = 1; // Já usamos um veículo para a rota herdada

    while(!clientesNaoAtendidos.empty() && veiculosUsados < dataCVRP.numVeiculos) {
        veiculosUsados++;
        filho.genes.push_back(0);
        double cargaAtual = 0.0;
        
        for(auto it = clientesNaoAtendidos.begin(); it != clientesNaoAtendidos.end(); ){
            int clienteAtual = *it;
            if(cargaAtual + dataCVRP.demandas[clienteAtual] <= dataCVRP.capacidade){
                filho.genes.push_back(clienteAtual);
                cargaAtual += dataCVRP.demandas[clienteAtual];
                it = clientesNaoAtendidos.erase(it);
            } else {
                ++it;
            }
        }
        filho.genes.push_back(0);
    }

    // Se ainda sobraram clientes, o crossover falhou em criar uma solução válida.
    // Uma estratégia simples é retornar um dos pais.
    if (!clientesNaoAtendidos.empty()) {
        return pai1;
    }

    filho.fitness = calcularFitness(filho.genes); // Calcula o fitness com a função correta
    return filho;
}