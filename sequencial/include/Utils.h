#pragma once
#include <vector>
#include <string>
#include "Individuo.h"
#include <random>
#include "Island.h"
std::vector<std::vector<double>> lerArquivo(const std::string& caminhoArquivo);
std::uint64_t lerArquivoSeed(const std::string& caminho, std::size_t inicio, std::size_t fim);
std::vector<Island> criarMalha(int numIslands, int seed);
