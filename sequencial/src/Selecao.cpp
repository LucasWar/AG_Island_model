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

typedef std::vector<Individuo> vectorIndiviudos;
typedef std::vector<Island> vetorIslands;

vectorIndiviudos GeneticAlgorithm::selecao(vectorIndiviudos &populacao, std::mt19937 &geradorLocal) {
    // A seleção por torneio está correta e pode ser mantida.
    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);
    vectorIndiviudos pais;
    pais.reserve(2);

    while (pais.size() < 2) {
        vectorIndiviudos candidatos;
        candidatos.reserve(2);
        for (int i = 0; i < 2; ++i)
            candidatos.push_back(populacao[dist(geradorLocal)]);

        auto melhor = *std::min_element(candidatos.begin(), candidatos.end(),
                                         [](const Individuo& a, const Individuo& b){ return a.fitness < b.fitness; });

        bool duplicado = false;
        for (const auto &p : pais) if (p.genes == melhor.genes) { duplicado = true; break; }
        if (!duplicado) pais.push_back(melhor);
    }
    return pais;
}

vectorIndiviudos GeneticAlgorithm::selecionarElite(vectorIndiviudos &populacao){
    // O elitismo também está correto e pode ser mantido.
    vectorIndiviudos copia = populacao;
    if (tamElite > copia.size()) return copia; // Evita erro se a elite for maior que a pop
    std::partial_sort(copia.begin(), copia.begin() + tamElite, copia.end(),
                      [](const Individuo& a, const Individuo& b){ return a.fitness < b.fitness; });
    
    copia.resize(tamElite);
    return copia;
}

void GeneticAlgorithm::executarGeracao(Island &ilha, std::uniform_real_distribution<double> &distLocal) {
    if (ilha.populacao.empty()) return; // Guarda de segurança

    vectorIndiviudos elite = selecionarElite(ilha.populacao);
    int numFilhos = tamPopulacao - elite.size();
    vectorIndiviudos novaPop;
    novaPop.reserve(tamPopulacao);

    for (int i = 0; i < numFilhos; ++i) {
        auto pais = selecao(ilha.populacao, ilha.geradorlocal);
        // ALTERADO: Chamando os operadores corretos de CVRP
        Individuo prole = crossoverStrategy->aplicar(pais[0], pais[1], ilha.geradorlocal, dataCVRP);
        if (distLocal(ilha.geradorlocal) < probMutacao)
            mutacaoCVRP(prole, ilha.geradorlocal);
        novaPop.push_back(prole);
    }
    novaPop.insert(novaPop.end(), elite.begin(), elite.end());
    ilha.populacao = std::move(novaPop);
}