#include "Utils.h"
#include "cvrpData.h"
#include <iostream>

bool verificarValidadeCVRP(const std::vector<int> &genes, CVRPData dataCVRP){
    if (genes.size() < 3) return false; // mínimo: 0, cliente, 0

    int veiculosUsados = 0;
    double cargaAtual = 0.0;
    std::vector<bool> clientesAtendidos(dataCVRP.distancias.size(), false);

    for (int gene : genes) {
        if (gene == 0) {
            if (cargaAtual > 0) {
                veiculosUsados++;
                cargaAtual = 0.0;
            }
        } else {
            if (gene < 0 || gene >= dataCVRP.distancias.size()) return false; // cliente inválido
            if (clientesAtendidos[gene]) return false; // cliente duplicado
            clientesAtendidos[gene] = true;

            cargaAtual += dataCVRP.demandas[gene];
            if (cargaAtual > dataCVRP.capacidade) return false; // capacidade estourada
        }
    }

    // Número de veículos não pode ultrapassar o limite
    if (veiculosUsados > dataCVRP.numVeiculos) return false;

    // Verifica se todos os clientes foram atendidos
    for (size_t c = 1; c < dataCVRP.distancias.size(); ++c) { // 0 = depósito
        if (!clientesAtendidos[c]) return false;
    }

    return true; // passou em todas as verificações
}



int main() {
    CVRPData dataCVRP;
    dataCVRP = lerArquivoVRP("entradas/B-n78-k10.txt");
    auto teste=verificarValidadeCVRP({0,61,39,59,0,6,66,0,42,18,10,68,53,64,77,0,17,72,56,22,12,34,45,31,7,47,0,29,35,55,33,13,19,14,41,60,16,25,0,76,32,73,20,54,3,49,0,8,71,15,46,51,57,75,30,0,65,5,2,40,11,26,44,23,0,62,50,48,74,28,9,36,0,58,38,69,67,43,21,24,52,1,0,63,70,37,27,0}, dataCVRP);
    std::cout << teste;
}