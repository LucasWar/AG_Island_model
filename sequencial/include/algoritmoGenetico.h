#pragma once
#include "Individuo.h"
#include "TypesUtils.h"
#include "cvrpData.h"
#include "Island.h"
#include "Reparar.h"
#include "Crossover.h"
#include <memory>
#include <vector>
#include <random>

class GeneticAlgorithm {
private:

    int numGeracoes, tamPopulacao, numNovosIndividuos, tamIndividuo,numInslands,freqMigracao,tamMigracao;
    float probMutacao,tamElite,numMigracao;
    CVRPData dataCVRP;
    std::string opcTopologia,opcCrossover;
    std::uint64_t seed;
    std::vector<std::vector<double>> matrizDeCaminhos;
    std::unique_ptr<ICrossover> crossoverStrategy; 
    vectorIndiviudos populacao;

    void executarGeracao(Island &ilha, std::uniform_real_distribution<double> &distLocal);
    
    void gerarPopulacaoCVPR(vetorIslands &islands);

    Individuo melhorIndividuo(vetorIslands &island, int geracao);

    vectorIndiviudos selecaoTorneio(vectorIndiviudos &populacao,std::mt19937 &geradorLocal);
    vectorIndiviudos selecaoRoleta(vectorIndiviudos &populacao,std::mt19937 &geradorLocal);

    // Individuo crossoverOX(const Individuo &pai1, const Individuo &pai2,std::mt19937 &geradorLocal);
    // Individuo crossoverPMX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    // Individuo crossoverCVRP(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    

    double calcularDistancia(const std::vector<int>& individuo) const;
    double calcularDistanciaDemanda(const std::vector<int> &individuo) const;
    
    void mutacao(Individuo &individuo,std::mt19937 &geradorLocal);
    void mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal);

    vectorIndiviudos selecionarMigrantesUnicos(const std::vector<Individuo>& populacao, std::mt19937& gerador);
    void realizarMigracao(vetorIslands &islands);
    void migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados);
    vectorIndiviudos selecionarMigrantes_Torneio(const vectorIndiviudos &populacao, std::mt19937 &gerador, int tamanhoTorneio);
    void realizarMigracao_Aprimorada(vetorIslands &islands);
    //std::vector<int> repararCVRP(const std::vector<int>& clientes);
    //std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes);

    void gerarPopulacaoDiversificada(vetorIslands &islands);
    
    std::vector<int> criarIndividuo_InsercaoAleatoria(std::mt19937& gerador);
    std::vector<int> criarIndividuo_VizinhoMaisProximo(std::mt19937& gerador);
    std::vector<int> criarIndividuo_AleatorioGulos(std::mt19937& gerador);
    void reiniciarPopulacoes(Island &ilha, int geracaoAtual);
    Individuo gerarIndividuoUnicoDiversificado(std::mt19937 &gerador);

public:
    GeneticAlgorithm(int numGeracoes, float probMutacao, int tamPopulacao,float tamElite, const std::string& caminhoArquivo, std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,std::string opcCrossover,int freqMigracao);
    void gerarPopulacao(vetorIslands &islands);
    vectorIndiviudos selecionarElite(vectorIndiviudos &atualPopulacao);
    void executarAlgoritmo();
};