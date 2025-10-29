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

vectorIndiviudos GeneticAlgorithm::selecaoTorneio(vectorIndiviudos &populacao, std::mt19937 &geradorLocal) {
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

    // --- 1. Seleciona elite ---
    vectorIndiviudos elite = selecionarElite(ilha.populacao);

    // --- 2. Se a elite já cobre a população, apenas mantém ---
    if (elite.size() >= tamPopulacao) {
        ilha.populacao = elite;
        return;
    }

    // --- 3. Adaptação da taxa de mutação (diversidade adaptativa) ---
    double diversidade = calcularDiversidade(ilha.populacao);
    double taxaMutacao = ilha.proMutacao;
    if (diversidade < 0.2) // baixa diversidade
        taxaMutacao *= 1.5;
    else if (diversidade > 0.5)
        taxaMutacao *= 0.8;
    taxaMutacao = std::clamp(taxaMutacao, 0.001, 0.5);

    // --- 4. Geração dos filhos ---
    vectorIndiviudos novaPop;
    novaPop.reserve(tamPopulacao);
    novaPop.insert(novaPop.end(), elite.begin(), elite.end());

    const int numFilhos = tamPopulacao - elite.size();

    for (int i = 0; i < numFilhos; ++i) {
        auto pais = (ilha.tipoSelecao == "Roleta") ?
                    selecaoRoleta(ilha.populacao, ilha.geradorlocal) :
                    selecaoTorneio(ilha.populacao, ilha.geradorlocal);

        Individuo filho = ilha.crossoverilha->aplicar(
            pais[0], pais[1], ilha.geradorlocal, dataCVRP, ilha.usaBuscaLocal);

        // Mutação com probabilidade adaptada
        if (distLocal(ilha.geradorlocal) < taxaMutacao) {
            mutacaoCVRP(filho, ilha.geradorlocal);
        }

        // Mutação extra em caso de duplicatas (evita clones)
        bool duplicado = std::any_of(novaPop.begin(), novaPop.end(),
            [&](const Individuo &ind) { return ind.genes == filho.genes; });
        if (duplicado && distLocal(ilha.geradorlocal) < 0.3)
            mutacaoCVRP(filho, ilha.geradorlocal);

        novaPop.push_back(std::move(filho));
    }

    // --- 5. Atualiza população ---
    ilha.populacao = std::move(novaPop);
}



double GeneticAlgorithm::calcularDiversidade(const vectorIndiviudos &pop) {
    if (pop.empty()) return 0.0;

    // Hash simples para medir diversidade genética
    std::unordered_set<size_t> genesUnicos;
    for (const auto &ind : pop)
        genesUnicos.insert(hashGenes(ind.genes));

    return static_cast<double>(genesUnicos.size()) / pop.size();
}