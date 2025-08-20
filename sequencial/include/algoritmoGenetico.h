#pragma once
#include <vector>
#include <random>
#include "Individuo.h"
#include "Island.h"
#include "cvrpData.h"
class GeneticAlgorithm {
private:
    typedef std::vector<Individuo> vectorIndiviudos;
    typedef std::vector<Island> vetorIslands;

    int numGeracoes, tamPopulacao, numNovosIndividuos, tamIndividuo,numInslands,freqMigracao,tamMigracao;
    float probMutacao,tamElite,numMigracao;
    CVRPData dataCVRP;

    std::string topologia;
    std::uint64_t seed;
    std::vector<std::vector<double>> matrizDeCaminhos;
    vectorIndiviudos populacao;
    // std::mt19937 gerador;

    vectorIndiviudos selecao(vectorIndiviudos &populacao,std::mt19937 &geradorLocal);
    
    Individuo melhorIndividuo(vetorIslands &islands);
    double calcularDistancia(const std::vector<int>& individuo) const;
    
    void mutacao(Individuo &individuo,std::mt19937 &geradorLocal);
    Individuo crossoverOX(const Individuo &pai1, const Individuo &pai2,std::mt19937 &geradorLocal);

    vectorIndiviudos selecionarMigrantesUnicos(const std::vector<Individuo>& populacao, std::mt19937& gerador);
    void realizarMigracao(vetorIslands &islands);
    void migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados);
    void executarGeracao(Island &ilha, std::uniform_real_distribution<double> &distLocal);

    void gerarPopulacaoCVPR(vetorIslands &islands);
    double calcularDistanciaDemanda(const std::vector<int> &individuo) const;

    Individuo crossoverCVRP(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    void mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal);
    double calcularFitness(const std::vector<int> &rota) const;
public:
    GeneticAlgorithm(int numGeracoes, float probMutacao, int tamPopulacao,float tamElite, const std::string& caminhoArquivo, std::uint64_t seed, int numInslands,float numMigracao,std::string topologia,int freqMigracao);
    void gerarPopulacao(vetorIslands &islands);
    vectorIndiviudos selecionarElite(vectorIndiviudos &atualPopulacao);
    void executarAlgoritmo();
};