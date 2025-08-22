#include "algoritmoGenetico.h"
#include "cvrpData.h"
#include "TypesUtils.h"
#include <algorithm>
#include <unordered_set>


size_t hashGenes(const std::vector<int>& genes) {
    size_t seed = genes.size();
    for (int g : genes) {
        seed ^= std::hash<int>()(g) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

vectorIndiviudos GeneticAlgorithm::selecionarMigrantesUnicos(const vectorIndiviudos &populacao, std::mt19937 &gerador) {
    vectorIndiviudos selecionados;
    std::unordered_set<size_t> hashesVistos;

    std::vector<int> indices(populacao.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), gerador);

    for (int idx : indices) {
        const auto &ind = populacao[idx];
        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            hashesVistos.insert(h);
            selecionados.push_back(ind);
        }
        if (selecionados.size() >= tamMigracao) break;
    }
    return selecionados;
}

void GeneticAlgorithm::migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados) {
    int k = selecionados.size();
    if (k == 0 || populacao.empty()) return;

    if (k > populacao.size()) k = populacao.size();
    std::partial_sort(populacao.begin(), populacao.begin() + k, populacao.end(),
                      [](const Individuo &a, const Individuo &b){ return a.fitness > b.fitness; });

    for (int i = 0; i < k; ++i)
        populacao[i] = selecionados[i];
}

void GeneticAlgorithm::realizarMigracao(vetorIslands &islands) {
    for (auto &ilhaOrigem : islands) {
        auto selecionados = selecionarMigrantesUnicos(ilhaOrigem.populacao, ilhaOrigem.geradorlocal);
        for (int vizinhoId : ilhaOrigem.vizinhos) {
            auto it = std::find_if(islands.begin(), islands.end(),
                                     [vizinhoId](const Island &il){ return il.idIlha == vizinhoId; });
            if (it != islands.end())
                migracaoPopulacao(it->populacao, selecionados);
        }
    }
}