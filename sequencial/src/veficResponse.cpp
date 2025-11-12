#include "Utils.h"
#include "cvrpData.h"
#include <iostream>

// bool verificarValidadeCVRP(const std::vector<int> &genes, CVRPData dataCVRP){
//     if (genes.size() < 3) return false; // mínimo: 0, cliente, 0

//     int veiculosUsados = 0;
//     double cargaAtual = 0.0;
//     std::vector<bool> clientesAtendidos(dataCVRP.distancias.size(), false);

//     for (int gene : genes) {
//         if (gene == 0) {
//             if (cargaAtual > 0) {
//                 veiculosUsados++;
//                 cargaAtual = 0.0;
//             }
//         } else {
//             if (gene < 0 || gene >= dataCVRP.distancias.size()) return false; // cliente inválido
//             if (clientesAtendidos[gene]) return false; // cliente duplicado
//             clientesAtendidos[gene] = true;

//             cargaAtual += dataCVRP.demandas[gene];
//             if (cargaAtual > dataCVRP.capacidade) return false; // capacidade estourada
//         }
//     }

//     // Número de veículos não pode ultrapassar o limite
//     if (veiculosUsados > dataCVRP.numVeiculos) return false;

//     // Verifica se todos os clientes foram atendidos
//     for (size_t c = 1; c < dataCVRP.distancias.size(); ++c) { // 0 = depósito
//         if (!clientesAtendidos[c]) return false;
//     }

//     return true; // passou em todas as verificações
// }

double calcularFitness(const std::vector<int> &rota, CVRPData dataCVRP){
    double soma = 0.0;
    if (rota.size() < 2) return 1e9; // Rota inválida

    for (size_t i = 0; i < rota.size() - 1; ++i) {
        double atual = rota[i];
        double prox = rota[i + 1];

        if (atual >= dataCVRP.distancias.size() || prox >= dataCVRP.distancias.size() || atual < 0 || prox < 0) {
             return 1e9; // Penalidade por ID de cliente inválido
        }
        
        soma += dataCVRP.distancias[atual][prox];
    }
    return soma;
}

int main() {
    CVRPData dataCVRP;
    dataCVRP = lerArquivoVRP("entradas/M-n151-k12.txt");
    int total = calcularFitness({0,31,10,108,131,32,90,63,126,62,148,88,127,27,0,28,76,116,77,3,121,29,24,134,150,80,68,12,0,94,92,98,37,100,91,85,93,59,95,13,0,111,50,102,33,81,120,9,103,51,122,1,132,0,18,114,46,124,47,36,143,49,64,11,107,19,123,7,146,0,89,118,60,83,125,45,8,82,48,106,52,0,105,53,112,0,138,109,54,130,55,25,67,39,139,4,110,149,26,0,117,97,87,42,142,14,119,44,141,16,61,104,99,96,6,0,58,144,57,15,43,38,140,86,113,17,84,5,147,0,137,2,115,145,41,22,133,75,23,56,74,72,73,21,40,0,69,101,70,30,20,128,66,71,65,136,35,135,34,78,129,79,0},dataCVRP);
    auto validade=verificarValidadeCVRP({0,31,10,108,131,32,90,63,126,62,148,88,127,27,0,28,76,116,77,3,121,29,24,134,150,80,68,12,0,94,92,98,37,100,91,85,93,59,95,13,0,111,50,102,33,81,120,9,103,51,122,1,132,0,18,114,46,124,47,36,143,49,64,11,107,19,123,7,146,0,89,118,60,83,125,45,8,82,48,106,52,0,105,53,112,0,138,109,54,130,55,25,67,39,139,4,110,149,26,0,117,97,87,42,142,14,119,44,141,16,61,104,99,96,6,0,58,144,57,15,43,38,140,86,113,17,84,5,147,0,137,2,115,145,41,22,133,75,23,56,74,72,73,21,40,0,69,101,70,30,20,128,66,71,65,136,35,135,34,78,129,79,0}, dataCVRP);
    std::cout << validade;
    std::cout << total;
}