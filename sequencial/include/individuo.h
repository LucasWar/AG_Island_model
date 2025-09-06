#pragma once
#include <vector>
#include <string> 

struct Individuo {
    std::vector<int> genes;
    int fitness;

    Individuo();
    Individuo(std::vector<int> gen, double fit);
    std::string cromossomo();
};