#pragma once
#include "Individuo.h"
#include "cvrpData.h"
#include "Ilha.h"
#include <vector>
#include <string>
#include <random>
std::vector<std::vector<double>> lerArquivo(const std::string& caminhoArquivo);
std::uint64_t lerArquivoSeed(const std::string& caminho, std::size_t inicio, std::size_t fim);
std::vector<Ilha> criarMalha(int numIlhas, int seed);
std::vector<Ilha> criarAnel(int numIlhas, int seed);
CVRPData lerArquivoVRP(const std::string& caminhoArquivo);
void printVector(const std::vector<int>& vec, const std::string& label = "");
bool verificarValidadeCVRP(const std::vector<int> &genes, CVRPData dataCVRP);
size_t hashGenes(const std::vector<int>& genes);
void salvarResultados(const Individuo& melhor, int solucaoOtima, int duracao, int numGerSemEvo, const std::vector<int>& genes, const CVRPData& dataCVRP, int numIlhas, std::string opcTopologia);
std::vector<int> extrairClientes(const std::vector<int>& genes);
std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes);
std::pair<int, int> sortearPontosCorte(int tamanho, std::mt19937& gerador);
