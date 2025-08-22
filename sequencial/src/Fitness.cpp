#include "algoritmoGenetico.h"

double GeneticAlgorithm::calcularFitness(const std::vector<int> &rota){
    double soma = 0.0;
    if (rota.size() < 2) return 1e9; // Rota inválida

    for (size_t i = 0; i < rota.size() - 1; ++i) {
        int atual = rota[i];
        int prox = rota[i + 1];

        if (atual >= dataCVRP.distancias.size() || prox >= dataCVRP.distancias.size() || atual < 0 || prox < 0) {
             return 1e9; // Penalidade por ID de cliente inválido
        }
        
        soma += dataCVRP.distancias[atual][prox];
    }
    return soma;
}