#include "algoritmoGenetico.h"
#include "Fitness.h"
#include <algorithm>
#include <list>
#include <iostream>

std::vector<int> GeneticAlgorithm::criarIndividuo_AleatorioGulos(std::mt19937 &geradorLocal) {
    std::vector<int> clientes(tamIndividuo - 1);
    std::iota(clientes.begin(), clientes.end(), 1);
    std::shuffle(clientes.begin(), clientes.end(), geradorLocal); // Usando um gerador global ou passado

    std::vector<int> genes;
    std::list<int> clientesNaoAtendidos(clientes.begin(), clientes.end());

    int veiculosUsados = 0;
    while (!clientesNaoAtendidos.empty() && veiculosUsados < dataCVRP.numVeiculos) {
        veiculosUsados++;
        genes.push_back(0); // Inicia nova rota
        double cargaAtual = 0.0;

        for (auto it = clientesNaoAtendidos.begin(); it != clientesNaoAtendidos.end();) {
            int clienteAtual = *it;
            if (cargaAtual + dataCVRP.demandas[clienteAtual] <= dataCVRP.capacidade) {
                genes.push_back(clienteAtual);
                cargaAtual += dataCVRP.demandas[clienteAtual];
                it = clientesNaoAtendidos.erase(it);
            } else {
                ++it;
            }
        }
        genes.push_back(0); // Fecha rota
    }

    if (!clientesNaoAtendidos.empty()) {
        return {}; // Falha na criação
    }
    return genes;
}

// Retorna um indivíduo ou um vetor vazio se falhar
std::vector<int> GeneticAlgorithm::criarIndividuo_VizinhoMaisProximo(std::mt19937 &geradorLocal) {
    std::vector<int> genes;
    std::vector<bool> visitado(tamIndividuo, false);
    int clientesAtendidos = 0;
    int veiculosUsados = 0;

    while (clientesAtendidos < tamIndividuo - 1 && veiculosUsados < dataCVRP.numVeiculos) {
        veiculosUsados++;
        genes.push_back(0); // Inicia nova rota

        // Escolhe um ponto de partida aleatório entre os não visitados
        int primeiroCliente = -1;
        std::vector<int> indicesClientes;
        for(int i = 1; i < tamIndividuo; ++i) if(!visitado[i]) indicesClientes.push_back(i);
        std::shuffle(indicesClientes.begin(), indicesClientes.end(), geradorLocal);
        if(indicesClientes.empty()) break;
        primeiroCliente = indicesClientes[0];
        
        double cargaAtual = dataCVRP.demandas[primeiroCliente];
        genes.push_back(primeiroCliente);
        visitado[primeiroCliente] = true;
        clientesAtendidos++;
        int ultimoCliente = primeiroCliente;

        bool rotaPodeCrescer = true;
        while (rotaPodeCrescer) {
            int vizinhoMaisProximo = -1;
            double menorDistancia = std::numeric_limits<double>::max();

            // Encontra o vizinho mais próximo e não visitado
            for (int i = 1; i < tamIndividuo; ++i) {
                if (!visitado[i] && dataCVRP.distancias[ultimoCliente][i] < menorDistancia) {
                    vizinhoMaisProximo = i;
                    menorDistancia = dataCVRP.distancias[ultimoCliente][i];
                }
            }

            // Verifica se o vizinho encontrado cabe na rota
            if (vizinhoMaisProximo != -1 && cargaAtual + dataCVRP.demandas[vizinhoMaisProximo] <= dataCVRP.capacidade) {
                genes.push_back(vizinhoMaisProximo);
                visitado[vizinhoMaisProximo] = true;
                cargaAtual += dataCVRP.demandas[vizinhoMaisProximo];
                ultimoCliente = vizinhoMaisProximo;
                clientesAtendidos++;
            } else {
                rotaPodeCrescer = false; // Rota cheia ou não há mais vizinhos
            }
        }
        genes.push_back(0); // Fecha rota
    }
    
    if (clientesAtendidos < tamIndividuo - 1) {
        return {}; // Falha na criação
    }
    return genes;
}

// Retorna um indivíduo ou um vetor vazio se falhar
std::vector<int> GeneticAlgorithm::criarIndividuo_InsercaoAleatoria(std::mt19937 &geradorLocal) {
    std::vector<std::vector<int>> rotas(dataCVRP.numVeiculos);
    std::vector<double> cargas(dataCVRP.numVeiculos, 0.0);

    std::vector<int> clientes(tamIndividuo - 1);
    std::iota(clientes.begin(), clientes.end(), 1);
    std::shuffle(clientes.begin(), clientes.end(), geradorLocal);

    for (int cliente : clientes) {
        double demanda = dataCVRP.demandas[cliente];
        bool inserido = false;

        // Tenta inserir o cliente em uma rota aleatória que tenha capacidade
        std::vector<int> indicesRotas(dataCVRP.numVeiculos);
        std::iota(indicesRotas.begin(), indicesRotas.end(), 0);
        std::shuffle(indicesRotas.begin(), indicesRotas.end(), geradorLocal);

        for (int i : indicesRotas) {
            if (cargas[i] + demanda <= dataCVRP.capacidade) {
                rotas[i].push_back(cliente);
                cargas[i] += demanda;
                inserido = true;
                break;
            }
        }

        if (!inserido) {
            return {}; // Falha, não foi possível inserir o cliente
        }
    }

    // Constrói o vetor de genes a partir das rotas
    std::vector<int> genes;
    genes.push_back(0);
    for (const auto& rota : rotas) {
        if (!rota.empty()) {
            genes.insert(genes.end(), rota.begin(), rota.end());
            genes.push_back(0);
        }
    }
    // Remove o último 0 se a última rota não vazia já o adicionou
    if (genes.back() == 0 && genes.size() > 1) {
       // A estrutura de loop pode adicionar um 0 a mais no final, mas a formatação pode variar.
       // O ideal é garantir o formato 0...0...0.
    } else {
        genes.push_back(0); // Garante que a última rota seja fechada
    }


    return genes;
}


void GeneticAlgorithm::gerarPopulacaoDiversificada(vetorIslands &islands) {
    // Supondo que sua função de reparo se chame 'tentarRepararSolucaoCVRP'
    // std::vector<int> genesReparados = tentarRepararSolucaoCVRP(genes);
    
    for (auto &island : islands) {
        island.populacao.reserve(tamPopulacao);
        for (int i = 0; i < tamPopulacao; ) {
            std::vector<int> genes;

            // Define a estratégia de geração baseada na proporção
            double proporcao = static_cast<double>(i) / tamPopulacao;

            if (proporcao < 0.4) { // 40% da população com Aleatorio-Gulos
                genes = criarIndividuo_AleatorioGulos(island.geradorlocal);
            } else if (proporcao < 0.8) { // 40% com Vizinho Mais Próximo
                genes = criarIndividuo_VizinhoMaisProximo(island.geradorlocal);
            } else { // 20% com Inserção Aleatória
                genes = criarIndividuo_InsercaoAleatoria(island.geradorlocal);
            }

            // Se a heurística falhou em gerar um indivíduo completo, tenta de novo
            if (genes.empty()) {
                continue; // O loop 'for' não incrementa 'i'
            }
            
            genes = repararCVRP(genes,dataCVRP,island.usaBuscaLocal);

            if(!genes.empty()){
                Individuo ind(genes, calcularFitness(genes, dataCVRP));
                island.populacao.push_back(ind);
                i++; // Só incrementa quando um indivíduo válido é adicionado
            }
        }
    }
}


void GeneticAlgorithm::gerarPopulacao(vetorIslands &islands){
    for(auto &island : islands){
        island.populacao.reserve(tamPopulacao);
        for(int i = 0; i < tamPopulacao; ){ 
            
            std::vector<int> clientes(tamIndividuo - 1);
            std::iota(clientes.begin(), clientes.end(), 1);
            std::shuffle(clientes.begin(), clientes.end(), island.geradorlocal);

            std::vector<int> genes;
            std::list<int> clientesNaoAtendidos(clientes.begin(), clientes.end());

            int veiculosUsados = 0;
            while(!clientesNaoAtendidos.empty() && veiculosUsados < dataCVRP.numVeiculos) {
                veiculosUsados++;
                genes.push_back(0); // Inicia uma nova rota
                double cargaAtual = 0.0;
                
                for(auto it = clientesNaoAtendidos.begin(); it != clientesNaoAtendidos.end(); ){
                    int clienteAtual = *it;
                    if(cargaAtual + dataCVRP.demandas[clienteAtual] <= dataCVRP.capacidade){
                        genes.push_back(clienteAtual);
                        cargaAtual += dataCVRP.demandas[clienteAtual];
                        it = clientesNaoAtendidos.erase(it);
                    } else {
                        ++it;
                    }
                }
                genes.push_back(0); // Fecha a rota atual
            }

            if (!clientesNaoAtendidos.empty()) {
                // Tenta gerar o indivíduo 'i' novamente se a permutação não foi viável
                continue;
            }

            Individuo ind(genes, calcularFitness(genes,dataCVRP));
            island.populacao.push_back(ind);
            i++; // Indivíduo válido gerado, incrementa
        }
    }
}