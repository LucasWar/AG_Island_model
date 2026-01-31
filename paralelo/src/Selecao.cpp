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
typedef std::vector<Ilha> vetorIlhas;

vectorIndiviudos GeneticAlgorithm::selecaoTorneio(vectorIndiviudos &populacao, std::mt19937 &geradorLocal, int tamTorneio) {
    // A seleção por torneio está correta e pode ser mantida.
    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);
    vectorIndiviudos pais;
    pais.reserve(2);

    while (pais.size() < 2) {
        vectorIndiviudos candidatos;
        candidatos.reserve(2);
        for (int i = 0; i < tamTorneio; ++i)
            candidatos.push_back(populacao[dist(geradorLocal)]);

        auto melhor = *std::min_element(candidatos.begin(), candidatos.end(),
                                         [](const Individuo& a, const Individuo& b){ return a.fitness < b.fitness; });

        bool duplicado = false;
        for (const auto &p : pais) if (p.genes == melhor.genes) { duplicado = true; break; }
        if (!duplicado) pais.push_back(melhor);
    }
    return pais;
}



vectorIndiviudos GeneticAlgorithm::selecaoRoleta(vectorIndiviudos& populacao, std::mt19937& gen) {
    std::vector<Individuo> pais(2);
    double totalFitness = 0.0;
    for (const auto& ind : populacao)
        totalFitness += 1.0 / ind.fitness; // quanto menor o fitness, melhor

    std::uniform_real_distribution<double> dist(0.0, totalFitness);

    for (int p = 0; p < 2; ++p) {
        double r = dist(gen);
        double acumulado = 0.0;
        for (const auto& ind : populacao) {
            acumulado += 1.0 / ind.fitness;
            if (acumulado >= r) {
                pais[p] = ind;
                break;
            }
        }
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


void GeneticAlgorithm::executarGeracao(Ilha &ilha, std::uniform_real_distribution<double> &distLocal) {
    if (ilha.populacao.empty()) return;

    const int tamPop = tamPopulacao;

    // --- 1. Seleção da elite ---
    vectorIndiviudos elite = selecionarElite(ilha.populacao);
    if (elite.size() >= tamPop) {
        ilha.populacao = std::move(elite);
        return;
    }

    // --- 2. Diversidade adaptativa ---
    double diversidade = 0.0;
    static const int kDiversidade = 5; // a cada 5 gerações
    if (ilha.geracaoUltimaEvolucao % kDiversidade == 0)
        diversidade = calcularDiversidade(ilha.populacao);

    double taxaMutacao = ilha.proMutacao;
    if (diversidade < 0.2) taxaMutacao *= 1.5;
    else if (diversidade > 0.5) taxaMutacao *= 0.8;
    taxaMutacao = std::clamp(taxaMutacao, 0.001, 0.5);

    // --- 3. Preparar população nova ---
    vectorIndiviudos novaPop;
    novaPop.reserve(tamPop);
    novaPop.insert(novaPop.end(), elite.begin(), elite.end());

    std::unordered_set<size_t> hashesPop;
    for (const auto &ind : novaPop)
        hashesPop.insert(hashGenes(ind.genes));

    // --- 4. Pré-calcular inverso de fitness para roleta ---
    std::vector<double> invFitness;
    if (ilha.tipoSelecao == "Roleta") {
        invFitness.reserve(ilha.populacao.size());
        for (auto &ind : ilha.populacao)
            invFitness.push_back(1.0 / ind.fitness);
    }

    const int numFilhos = tamPop - static_cast<int>(novaPop.size());

    // --- 5. Loop de criação dos filhos (sequencial) ---
    for (int i = 0; i < numFilhos; ++i) {
        vectorIndiviudos pais = (ilha.tipoSelecao == "Roleta") ?
                                 selecaoRoleta(ilha.populacao, ilha.geradorlocal) :
                                 selecaoTorneio(ilha.populacao, ilha.geradorlocal, ilha.tamanhoTorneio);

        Individuo filho = ilha.crossoverilha->aplicar(
            pais[0], pais[1], ilha.geradorlocal, dataCVRP, ilha.usaBuscaLocal
        );

        if (distLocal(ilha.geradorlocal) < taxaMutacao)
            mutacaoCVRP(filho, ilha.geradorlocal);

        size_t hashFilho = hashGenes(filho.genes);
        if (hashesPop.count(hashFilho) && distLocal(ilha.geradorlocal) < 0.3)
            mutacaoCVRP(filho, ilha.geradorlocal);

        novaPop.push_back(std::move(filho));
        hashesPop.insert(hashFilho);
    }

    // --- 6. Atualiza a população da ilha ---
    ilha.populacao = std::move(novaPop);
    ilha.geracaoUltimaEvolucao++;
}


double GeneticAlgorithm::calcularDiversidade(const vectorIndiviudos &pop) {
    if (pop.empty()) return 0.0;

    // Hash simples para medir diversidade genética
    std::unordered_set<size_t> genesUnicos;
    for (const auto &ind : pop)
        genesUnicos.insert(hashGenes(ind.genes));

    return static_cast<double>(genesUnicos.size()) / pop.size();
}