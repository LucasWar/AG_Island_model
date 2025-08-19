#include "Individuo.h"
#include <sstream>

Individuo::Individuo() {}

Individuo::Individuo(std::vector<int> gen, double fit) : genes(gen),fitness(fit) {}

std::string Individuo::cromossomo() {
    std::ostringstream oss;
    for (size_t i = 0; i < genes.size(); ++i) {
        oss << genes[i];
        if (i != genes.size() - 1) {
            oss << "-";
        }
    }
    return oss.str();
}