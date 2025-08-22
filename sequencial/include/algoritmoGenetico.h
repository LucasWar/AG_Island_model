#pragma once
#include "Individuo.h"
#include "TypesUtils.h"
#include "cvrpData.h"
#include "Island.h"
#include <vector>
#include <random>

class GeneticAlgorithm {
private:

    int numGeracoes, tamPopulacao, numNovosIndividuos, tamIndividuo,numInslands,freqMigracao,tamMigracao;
    float probMutacao,tamElite,numMigracao;
    CVRPData dataCVRP;

    std::string opcTopologia;
    std::uint64_t seed;
    std::vector<std::vector<double>> matrizDeCaminhos;
    vectorIndiviudos populacao;

    void executarGeracao(Island &ilha, std::uniform_real_distribution<double> &distLocal);
    
    void gerarPopulacaoCVPR(vetorIslands &islands);

    Individuo melhorIndividuo(vetorIslands &islands);

    vectorIndiviudos selecao(vectorIndiviudos &populacao,std::mt19937 &geradorLocal);

    Individuo crossoverOX(const Individuo &pai1, const Individuo &pai2,std::mt19937 &geradorLocal);
    Individuo crossoverPMX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    Individuo crossoverCVRP(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    
    double calcularDistancia(const std::vector<int>& individuo) const;
    double calcularFitness(const std::vector<int> &rota);
    //double calcularFitness(const std::vector<int> &rota) const;
    double calcularDistanciaDemanda(const std::vector<int> &individuo) const;
    
    void mutacao(Individuo &individuo,std::mt19937 &geradorLocal);
    void mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal);

    vectorIndiviudos selecionarMigrantesUnicos(const std::vector<Individuo>& populacao, std::mt19937& gerador);
    void realizarMigracao(vetorIslands &islands);
    void migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados);
   
    std::vector<int> repararCVRP(const std::vector<int>& clientes);
    std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes);
public:
    GeneticAlgorithm(int numGeracoes, float probMutacao, int tamPopulacao,float tamElite, const std::string& caminhoArquivo, std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,int freqMigracao);
    void gerarPopulacao(vetorIslands &islands);
    vectorIndiviudos selecionarElite(vectorIndiviudos &atualPopulacao);
    void executarAlgoritmo();
};