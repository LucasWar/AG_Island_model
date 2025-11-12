#pragma once
#include "Individuo.h"
#include "TypesUtils.h"
#include "cvrpData.h"
#include "Ilha.h"
#include "Reparar.h"
#include "Crossover.h"
#include <memory>
#include <vector>
#include <random>
#include <unordered_set>
class GeneticAlgorithm {
private:

    int numGeracoes, tamPopulacao, numNovosIndividuos, tamIndividuo,numInslands,freqMigracao,tamMigracao;
    float probMutacao,tamElite,numMigracao;
    CVRPData dataCVRP;
    std::string opcTopologia,opcCrossover, opcSelecao;
    std::uint64_t seed;
    std::vector<std::vector<double>> matrizDeCaminhos;
    std::unique_ptr<ICrossover> crossoverStrategy; 
    vectorIndiviudos populacao;

    void executarGeracao(Ilha &ilha, std::uniform_real_distribution<double> &distLocal);
    
    void gerarPopulacaoCVPR(vetorIlhas &ilhas);

    Individuo melhorIndividuo(vetorIlhas &ilha, int geracao);

    vectorIndiviudos selecaoTorneio(vectorIndiviudos &populacao,std::mt19937 &geradorLocal,int tamTorneio);
    vectorIndiviudos selecaoRoleta(vectorIndiviudos &populacao,std::mt19937 &geradorLocal);

    // Individuo crossoverOX(const Individuo &pai1, const Individuo &pai2,std::mt19937 &geradorLocal);
    // Individuo crossoverPMX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    // Individuo crossoverCVRP(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal);
    

    double calcularDistancia(const std::vector<int>& individuo) const;
    double calcularDistanciaDemanda(const std::vector<int> &individuo) const;
    
    void mutacao(Individuo &individuo,std::mt19937 &geradorLocal);
    void mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal);

    vectorIndiviudos selecionarMigrantesUnicos(const std::vector<Individuo>& populacao, std::mt19937& gerador);
    void realizarMigracao(vetorIlhas &ilhas);
    void migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados);
    vectorIndiviudos selecionarMigrantes_Torneio(const vectorIndiviudos &populacao, std::mt19937 &gerador, int tamanhoTorneio, double parcelaPop);
    void realizarMigracao_Aprimorada(vetorIlhas &ilhas);
    //std::vector<int> repararCVRP(const std::vector<int>& clientes);
    //std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes);

    void gerarPopulacaoDiversificada(vetorIlhas &ilhas);
    
    std::vector<int> criarIndividuo_InsercaoAleatoria(std::mt19937& gerador);
    std::vector<int> criarIndividuo_VizinhoMaisProximo(std::mt19937& gerador);
    std::vector<int> criarIndividuo_AleatorioGulos(std::mt19937& gerador);
    //void reiniciarPopulacoes(Ilha &ilha, int geracaoAtual);
    Individuo gerarIndividuoUnicoDiversificado(std::mt19937 &gerador);
    void adicionarIndividuosAleatorios(Ilha &ilha, vectorIndiviudos &novaPopulacao, std::unordered_set<size_t> &hashesVistos, size_t numAleatorios);
    void preservarElite(Ilha &ilha, vectorIndiviudos &novaPopulacao, std::unordered_set<size_t> &hashesVistos, size_t numMelhores);
    void gerarIndividuosDiversificados(Ilha &ilha, vectorIndiviudos &novaPopulacao, size_t numAGerar);
    void atualizarMelhorIndividuo(Ilha &ilha, int geracaoAtual);
    size_t calcularQuantidadeAPreservar(size_t tamPopulacao, double percentual);

    vectorIndiviudos selecionarMigrantesMelhores(const vectorIndiviudos &populacao, const double parcelaPop);

    std::vector<size_t> escolherIlhasMigrantes(vetorIlhas &ilhas, double probMigrar);
    std::vector<vectorIndiviudos> coletarMigrantes(vetorIlhas &ilhas, const std::vector<size_t> &ilhasMigrantes,int tamTorneio);
    void enviarMigrantes(vetorIlhas &ilhas, const std::vector<size_t> &ilhasMigrantes, const std::vector<vectorIndiviudos> &migrantesPorIlha);

    std::vector<Ilha>  inicarAlgoritmo();
public:
    GeneticAlgorithm(int numGeracoes, float probMutacao, int tamPopulacao,float tamElite, const std::string& caminhoArquivo, std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,int freqMigracao,std::string opcCrossover = "None", std::string opcSelecao = "None");
    void gerarPopulacao(vetorIlhas &ilhas);
    vectorIndiviudos selecionarElite(vectorIndiviudos &atualPopulacao);
    void executarAlgoritmo();
    void verificarEexecutarResgateColonial(vetorIlhas &ilhas, int geracaoAtual);
    double calcularDiversidade(const vectorIndiviudos &pop);
    void reiniciarPopulacoes(Ilha &ilha, int geracaoAtual);

};