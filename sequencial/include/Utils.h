#pragma once
#include "Individuo.h"
#include "cvrpData.h"
#include "Island.h"
#include <vector>
#include <string>
#include <random>
std::vector<std::vector<double>> lerArquivo(const std::string& caminhoArquivo);
std::uint64_t lerArquivoSeed(const std::string& caminho, std::size_t inicio, std::size_t fim);
std::vector<Island> criarMalha(int numIslands, int seed);
std::vector<Island> criarAnel(int numIlhas, int seed);
CVRPData lerArquivoVRP(const std::string& caminhoArquivo);
void printVector(const std::vector<int>& vec, const std::string& label = "");
