#include "algoritmoGenetico.h"
#include "utils.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <numeric>
#include <cmath>

// =====================
// Tipos auxiliares
// =====================
typedef std::vector<Individuo> vectorIndiviudos;
typedef std::vector<Island> vetorIslands;

// =====================
// Construtor
// =====================
GeneticAlgorithm::GeneticAlgorithm(int nGen, float pMut, int tPop, float nElite, const std::string& caminho,std::uint64_t seed, int numInslands,float numMigracao,std::string topologia,int freqMigracao) 
    :seed(seed),numGeracoes(nGen), probMutacao(pMut), tamPopulacao(tPop),numInslands(numInslands),topologia(topologia),freqMigracao(freqMigracao), tamMigracao(static_cast<int>(std::floor(numMigracao * tamPopulacao))), tamElite(static_cast<int>(std::floor(nElite * tamPopulacao))){
    
    matrizDeCaminhos = lerArquivo(caminho);
    tamIndividuo = matrizDeCaminhos.empty() ? 0 : matrizDeCaminhos.size();
}

// =====================
// Funções auxiliares
// =====================

void printVector(const std::vector<int>& vec, const std::string& label = "") {
    if (!label.empty()) std::cout << label << ": ";
    std::cout << "[ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << " ]\n";
}

size_t hashGenes(const std::vector<int>& genes) {
    size_t seed = genes.size();
    for (int g : genes) {
        seed ^= std::hash<int>()(g) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

// =====================
// Funções do GeneticAlgorithm
// =====================

Individuo GeneticAlgorithm::melhorIndividuo(vetorIslands &islands) {
    bool inicializado = false;
    Individuo melhor;

    for (auto &island : islands) {
        if (island.populacao.empty()) continue;

        auto it = std::min_element(island.populacao.begin(), island.populacao.end(),
                                   [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

        if (!inicializado || it->fitness < melhor.fitness) {
            melhor = *it;
            inicializado = true;
        }
    }

    if (!inicializado) throw std::runtime_error("Nenhuma população encontrada para avaliar.");
    return melhor;
}

double GeneticAlgorithm::calcularDistancia(const std::vector<int> &individuo) const {
    double soma = 0;
    for (size_t i = 0; i < individuo.size(); ++i) {
        int atual = individuo[i] - 1;
        int prox = individuo[(i + 1) % individuo.size()] - 1;
        if (atual < 0 || prox < 0 || atual >= matrizDeCaminhos.size()) return 1e9;
        soma += matrizDeCaminhos[atual][prox];
    }
    return soma;
}

void GeneticAlgorithm::gerarPopulacao(vetorIslands &islands){
    for(auto &island : islands){
        island.populacao.reserve(tamPopulacao);
        for(int i = 0; i < tamPopulacao; i++){
            Individuo newIndividuo;
            newIndividuo.genes.resize(tamIndividuo);
            std::iota(newIndividuo.genes.begin(), newIndividuo.genes.end(), 1);
            std::shuffle(newIndividuo.genes.begin(), newIndividuo.genes.end(), island.geradorlocal);
            newIndividuo.fitness = calcularDistancia(newIndividuo.genes);
            island.populacao.push_back(newIndividuo);
        }
    }
}

// =====================
// Seleção, Elite, Crossover e Mutação
// =====================

vectorIndiviudos GeneticAlgorithm::selecao(vectorIndiviudos &populacao, std::mt19937 &geradorLocal) {
    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);
    vectorIndiviudos pais;

    while (pais.size() < 2) {
        vectorIndiviudos candidatos;
        for (int i = 0; i < 4; ++i)
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
    vectorIndiviudos copia = populacao;
    std::partial_sort(copia.begin(), copia.begin() + tamElite, copia.end(),
                      [](const Individuo& a, const Individuo& b){ return a.fitness < b.fitness; });

    vectorIndiviudos elite;
    std::unordered_set<std::string> vistos;

    for (const auto &ind : copia) {
        std::string hash;
        for (int gene : ind.genes) hash += std::to_string(gene) + ",";
        if (vistos.find(hash) == vistos.end()) {
            vistos.insert(hash);
            elite.push_back(ind);
        }
        if (elite.size() >= tamElite) break;
    }
    return elite;
}

void GeneticAlgorithm::mutacao(Individuo &ind, std::mt19937 &geradorLocal) {
    std::uniform_int_distribution<int> dist(0, tamIndividuo - 1);
    int a = dist(geradorLocal), b;
    do { b = dist(geradorLocal); } while (a == b);
    std::swap(ind.genes[a], ind.genes[b]);
    std::reverse(ind.genes.begin() + a, ind.genes.begin() + b);
    ind.fitness = calcularDistancia(ind.genes);
}

Individuo GeneticAlgorithm::crossoverOX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal) {
    std::uniform_int_distribution<int> dist(0, tamIndividuo - 1);
    int p1 = dist(geradorLocal), p2 = dist(geradorLocal);
    if (p1 > p2) std::swap(p1, p2);

    Individuo filho;
    filho.genes.resize(tamIndividuo, -1);
    std::unordered_set<int> usados;
    for (int i = p1; i < p2; ++i) {
        filho.genes[i] = pai1.genes[i];
        usados.insert(pai1.genes[i]);
    }

    int pos = 0;
    for (int gene : pai2.genes) {
        if (usados.count(gene)) continue;
        while (pos >= p1 && pos < p2) ++pos;
        filho.genes[pos++] = gene;
    }
    filho.fitness = calcularDistancia(filho.genes);
    return filho;
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

// =====================
// Funções de alto nível
// =====================

void GeneticAlgorithm::executarGeracao(Island &ilha, std::uniform_real_distribution<double> &distLocal) {
    vectorIndiviudos elite = selecionarElite(ilha.populacao);
    int numFilhos = tamPopulacao - tamElite;
    vectorIndiviudos novaPop(numFilhos);

    for (int i = 0; i < numFilhos; ++i) {
        auto pais = selecao(ilha.populacao, ilha.geradorlocal);
        Individuo prole = crossoverOX(pais[0], pais[1], ilha.geradorlocal);
        if (distLocal(ilha.geradorlocal) < probMutacao)
            mutacao(prole, ilha.geradorlocal);
        novaPop[i] = prole;
    }

    novaPop.insert(novaPop.end(), elite.begin(), elite.end());
    ilha.populacao = std::move(novaPop);
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

// =====================
// Função principal
// =====================

void GeneticAlgorithm::executarAlgoritmo() {
    std::uniform_real_distribution<double> distLocal(0, 1);
    std::vector<Island> islands = criarMalha(numInslands, seed);

    gerarPopulacao(islands);
    Individuo melhor = melhorIndividuo(islands);

    for (int geracao = 0; geracao < numGeracoes; ++geracao) {
        for (auto &ilha : islands)
            executarGeracao(ilha, distLocal);

        if (geracao % freqMigracao == 0)
            realizarMigracao(islands);

        auto auxMelhor = melhorIndividuo(islands);
        if (auxMelhor.fitness < melhor.fitness)
            melhor = auxMelhor;
    }

    std::cout << "Melhor fitness: " << melhor.fitness << std::endl;
    std::cout << "Solução: " << melhor.cromossomo() << std::endl;
}
