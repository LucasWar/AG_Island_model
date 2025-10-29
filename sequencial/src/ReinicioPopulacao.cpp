#include "algoritmoGenetico.h"
#include "Utils.h"
#include <unordered_set>
#include <algorithm>


void GeneticAlgorithm::reiniciarPopulacoes(Ilha &ilha, int geracaoAtual) {
    if (ilha.populacao.size() < 10) return;

    size_t numMelhores = calcularQuantidadeAPreservar(tamPopulacao, 0.15);
    size_t numAleatorios = calcularQuantidadeAPreservar(tamPopulacao, 0.15);
    size_t numAGerar = tamPopulacao - numMelhores - numAleatorios;

    vectorIndiviudos novaPopulacao;
    novaPopulacao.reserve(tamPopulacao);

    std::unordered_set<size_t> hashesVistos;
    preservarElite(ilha, novaPopulacao, hashesVistos, numMelhores);
    adicionarIndividuosAleatorios(ilha, novaPopulacao, hashesVistos, numAleatorios);
    gerarIndividuosDiversificados(ilha, novaPopulacao, numAGerar);

    ilha.populacao = std::move(novaPopulacao);
    atualizarMelhorIndividuo(ilha, geracaoAtual);
}

size_t GeneticAlgorithm::calcularQuantidadeAPreservar(size_t tamPopulacao, double percentual) {
    return static_cast<size_t>(std::floor(tamPopulacao * percentual));
}

void GeneticAlgorithm::preservarElite(Ilha &ilha, vectorIndiviudos &novaPopulacao, std::unordered_set<size_t> &hashesVistos, size_t numMelhores) {
    std::sort(ilha.populacao.begin(), ilha.populacao.end(), [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

    for (const auto& ind : ilha.populacao) {
        if (novaPopulacao.size() >= numMelhores) break;
        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            novaPopulacao.push_back(ind);
            hashesVistos.insert(h);
        }
    }
}

void GeneticAlgorithm::adicionarIndividuosAleatorios(Ilha &ilha, vectorIndiviudos &novaPopulacao, std::unordered_set<size_t> &hashesVistos, size_t numAleatorios) {
    std::vector<int> indicesCandidatos;
    for (size_t i = 0; i < ilha.populacao.size(); ++i) {
        size_t h = hashGenes(ilha.populacao[i].genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            indicesCandidatos.push_back(i);
        }
    }
    std::shuffle(indicesCandidatos.begin(), indicesCandidatos.end(), ilha.geradorlocal);

    for (int idx : indicesCandidatos) {
        if (novaPopulacao.size() >= numAleatorios) break;
        const auto& ind = ilha.populacao[idx];
        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            novaPopulacao.push_back(ind);
            hashesVistos.insert(h);
        }
    }
}

void GeneticAlgorithm::gerarIndividuosDiversificados(Ilha &ilha, vectorIndiviudos &novaPopulacao, size_t numAGerar) {
    for (size_t i = 0; i < numAGerar; ++i) {
        novaPopulacao.push_back(gerarIndividuoUnicoDiversificado(ilha.geradorlocal));
    }
}

void GeneticAlgorithm::atualizarMelhorIndividuo(Ilha &ilha, int geracaoAtual) {
    auto it = std::min_element(ilha.populacao.begin(), ilha.populacao.end(),
                               [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

    if (it != ilha.populacao.end()) {
        ilha.melhorIndividuoIlhaa = *it;
        ilha.geracaoUltimaEvolucao = geracaoAtual;
    }
}
