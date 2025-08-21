#include "algoritmoGenetico.h"
#include "Topologia.h"
#include "utils.h"
#include "cvrpData.h"
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include <numeric>
#include <cmath>
#include <list>
#include <map> // NOVO: Usado nos novos operadores

// =====================
// Tipos auxiliares
// =====================
typedef std::vector<Individuo> vectorIndiviudos;
typedef std::vector<Island> vetorIslands;

// =====================
// Construtor
// =====================
GeneticAlgorithm::GeneticAlgorithm(int nGen, float pMut, int tPop, float nElite, const std::string& caminho,std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,int freqMigracao) 
    :seed(seed),numGeracoes(nGen), probMutacao(pMut), tamPopulacao(tPop),numInslands(numInslands),opcTopologia(opcTopologia),freqMigracao(freqMigracao), tamMigracao(static_cast<int>(std::floor(numMigracao * tamPopulacao))), tamElite(static_cast<int>(std::floor(nElite * tamPopulacao))){
    dataCVRP = lerArquivoVRP(caminho);
    // REMOVIDO: matrizDeCaminhos e tamIndividuo baseado nela
    tamIndividuo = dataCVRP.distancias.empty() ? 0 : dataCVRP.distancias.size(); // tamIndividuo agora representa o N total de locais (depósito + clientes)
}

// =====================
// Funções auxiliares
// =====================

void printVector(const std::vector<int>& vec, const std::string& label = "") {
    if (!label.empty()) std::cout << label << ": ";
    std::cout << "[ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << " ]\n";
}

size_t hashGenes(const std::vector<int>& genes) {
    size_t seed = genes.size();
    for (int g : genes) {
        seed ^= std::hash<int>()(g) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

// NOVO: Função auxiliar para extrair rotas de um cromossomo
std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes) {
    std::vector<std::vector<int>> rotas;
    if (genes.empty()) return rotas;

    std::vector<int> rotaAtual;
    for (size_t i = 1; i < genes.size(); ++i) {
        if (genes[i] != 0) {
            rotaAtual.push_back(genes[i]);
        } else {
            if (!rotaAtual.empty()) {
                rotas.push_back(rotaAtual);
                rotaAtual.clear();
            }
        }
    }
    return rotas;
}


// =====================
// Funções do GeneticAlgorithm
// =====================

Individuo GeneticAlgorithm::melhorIndividuo(vetorIslands &islands) {
    bool inicializado = false;
    Individuo melhor;

    for (auto &island : islands) {
        if (island.populacao.empty()) continue;

        auto it = std::min_element(island.populacao.begin(), island.populacao.end(),
                                     [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

        if (!inicializado || it->fitness < melhor.fitness) {
            melhor = *it;
            inicializado = true;
        }
    }

    if (!inicializado) throw std::runtime_error("Nenhuma população encontrada para avaliar.");
    return melhor;
}


// REMOVIDO: calcularDistancia - era para TSP e causava inconsistências
// double GeneticAlgorithm::calcularDistancia(...)

// ALTERADO: Renomeada para ser a função de fitness padrão
double GeneticAlgorithm::calcularFitness(const std::vector<int> &rota) const {
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

// REMOVIDO: gerarPopulacao - era para TSP
// void GeneticAlgorithm::gerarPopulacao(...)

// ALTERADO: Renomeada para ser a função de geração padrão
void GeneticAlgorithm::gerarPopulacao(vetorIslands &islands){
    for(auto &island : islands){
        island.populacao.reserve(tamPopulacao);
        for(int i = 0; i < tamPopulacao; ){ // Incremento manual para garantir população válida
            
            // tamIndividuo-1 é o número de clientes
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

            Individuo ind(genes, calcularFitness(genes));
            island.populacao.push_back(ind);
            i++; // Indivíduo válido gerado, incrementa
        }
    }
}

// =====================
// Seleção, Elite, Crossover e Mutação (VERSÃO CVRP)
// =====================

vectorIndiviudos GeneticAlgorithm::selecao(vectorIndiviudos &populacao, std::mt19937 &geradorLocal) {
    // A seleção por torneio está correta e pode ser mantida.
    std::uniform_int_distribution<int> dist(0, populacao.size() - 1);
    vectorIndiviudos pais;
    pais.reserve(2);

    while (pais.size() < 2) {
        vectorIndiviudos candidatos;
        candidatos.reserve(4);
        for (int i = 0; i < 4; ++i)
            candidatos.push_back(populacao[dist(geradorLocal)]);

        auto melhor = *std::min_element(candidatos.begin(), candidatos.end(),
                                         [](const Individuo& a, const Individuo& b){ return a.fitness < b.fitness; });

        bool duplicado = false;
        for (const auto &p : pais) if (p.genes == melhor.genes) { duplicado = true; break; }
        if (!duplicado) pais.push_back(melhor);
    }
    return pais;
}

vectorIndiviudos GeneticAlgorithm::selecionarElite(vectorIndiviudos &populacao){
    // O elitismo também está correto e pode ser mantido.
    vectorIndiviudos copia = populacao;
    if (tamElite > copia.size()) return copia; // Evita erro se a elite for maior que a pop
    std::partial_sort(copia.begin(), copia.begin() + tamElite, copia.end(),
                      [](const Individuo& a, const Individuo& b){ return a.fitness < b.fitness; });
    
    copia.resize(tamElite);
    return copia;
}

// NOVO: Operador de mutação específico para CVRP
void GeneticAlgorithm::mutacaoCVRP(Individuo &ind, std::mt19937 &geradorLocal) {
    if (ind.genes.size() <= 3) return; // Não há o que mutar

    auto rotas = extrairRotas(ind.genes);
    if (rotas.empty()) return;

    std::uniform_int_distribution<int> dist_rotas(0, rotas.size() - 1);
    int idx_rota = dist_rotas(geradorLocal);
    
    // Mutação: Troca (Swap) de dois clientes DENTRO da mesma rota
    // Esta é uma das mutações mais seguras, pois não afeta a capacidade
    if (rotas[idx_rota].size() >= 2) {
        std::uniform_int_distribution<int> dist_clientes(0, rotas[idx_rota].size() - 1);
        int pos1 = dist_clientes(geradorLocal);
        int pos2 = dist_clientes(geradorLocal);
        while (pos1 == pos2) {
            pos2 = dist_clientes(geradorLocal);
        }
        std::swap(rotas[idx_rota][pos1], rotas[idx_rota][pos2]);
        //std::reverse(rotas[idx_rota].begin() + pos1, rotas[idx_rota].begin() + pos2);
    }

    // Reconstruir o cromossomo
    std::vector<int> novos_genes;
    novos_genes.push_back(0);
    for (const auto& rota : rotas) {
        for (int cliente : rota) {
            novos_genes.push_back(cliente);
        }
        novos_genes.push_back(0);
    }
    
    ind.genes = novos_genes;
    ind.fitness = calcularFitness(ind.genes); // Recalcula o fitness com a função correta
}

// NOVO: Crossover específico para CVRP (Route Based Crossover Simplificado)

std::vector<int> GeneticAlgorithm::repararCVRP(const std::vector<int>& clientes) {
    // --- FASE 1: CONSTRUÇÃO GULOSA (GARANTE CAPACIDADE) ---
    std::vector<std::vector<int>> rotas;
    std::vector<double> cargas;

    if (clientes.empty()) {
        return {0, 0};
    }
    
    // Inicia a primeira rota
    rotas.push_back({});
    cargas.push_back(0.0);

    for (int cliente : clientes) {
        double demanda = dataCVRP.demandas[cliente];

        // Tenta alocar na última rota criada
        if (cargas.back() + demanda <= dataCVRP.capacidade) {
            rotas.back().push_back(cliente);
            cargas.back() += demanda;
        } else {
            // Se não couber, cria uma nova rota para este cliente
            rotas.push_back({cliente});
            cargas.push_back(demanda);
        }
    }

    // --- FASE 2: REPARO (GARANTE NÚMERO DE VEÍCULOS) ---
    if (rotas.size() <= dataCVRP.numVeiculos) {
        // Solução já é viável, apenas formate a saída
        std::vector<int> solucaoFinal;
        solucaoFinal.push_back(0);
        for (const auto& rota : rotas) {
            solucaoFinal.insert(solucaoFinal.end(), rota.begin(), rota.end());
            solucaoFinal.push_back(0);
        }
        return solucaoFinal;
    }

    // Se excedeu o número de veículos, precisamos reparar
    std::vector<int> clientesOrfaos;
    
    // Coleta clientes das rotas extras
    while (rotas.size() > dataCVRP.numVeiculos) {
        clientesOrfaos.insert(clientesOrfaos.end(), rotas.back().begin(), rotas.back().end());
        rotas.pop_back();
        cargas.pop_back();
    }

    // Tenta reinserir os clientes órfãos nas rotas válidas
    bool todosInseridos = true;
    for (int orfao : clientesOrfaos) {
        double demandaOrfao = dataCVRP.demandas[orfao];
        bool inserido = false;

        // Procura a melhor posição de inserção (aqui, a primeira que couber)
        // Uma melhoria seria buscar a inserção que gerasse o menor custo adicional.
        for (int i = 0; i < rotas.size(); ++i) {
            if (cargas[i] + demandaOrfao <= dataCVRP.capacidade) {
                rotas[i].push_back(orfao); // Inserção simples no final
                cargas[i] += demandaOrfao;
                inserido = true;
                break; // Vai para o próximo órfão
            }
        }
        
        if (!inserido) {
            todosInseridos = false;
            break; // Se um órfão não puder ser inserido, a reparação falhou
        }
    }

    if (todosInseridos) {
        // Sucesso na reparação! Formate a saída.
        std::vector<int> solucaoFinal;
        solucaoFinal.push_back(0);
        for (const auto& rota : rotas) {
            solucaoFinal.insert(solucaoFinal.end(), rota.begin(), rota.end());
            solucaoFinal.push_back(0);
        }
        return solucaoFinal;
    } else {
        // Reparação impossível para esta permutação de clientes.
        // Retornar um vetor vazio sinaliza a falha.
        // O Algoritmo Genético deve então penalizar fortemente esta solução.
        return {}; 
    }
}


Individuo GeneticAlgorithm::crossoverOX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal) {
    // --- Extrair só os clientes (ignorar zeros) ---
    std::vector<int> clientes1, clientes2;
    for (int g : pai1.genes) if (g != 0) clientes1.push_back(g);
    for (int g : pai2.genes) if (g != 0) clientes2.push_back(g);

    int n = clientes1.size();
    std::uniform_int_distribution<int> dist(0, n - 1);
    int cut1 = dist(geradorLocal);
    int cut2 = dist(geradorLocal);
    if (cut1 > cut2) std::swap(cut1, cut2);

    std::vector<int> filho(n, -1);

    // Copiar segmento do pai1
    for (int i = cut1; i <= cut2; i++) {
        filho[i] = clientes1[i];
    }

    // Preencher com clientes do pai2 na ordem
    int pos = (cut2 + 1) % n;
    for (int c : clientes2) {
        if (std::find(filho.begin(), filho.end(), c) == filho.end()) {
            filho[pos] = c;
            pos = (pos + 1) % n;
        }
    }

    // --- Neste ponto "filho" é uma permutação de clientes, SEM rotas ---
    // Agora precisamos reconstruir rotas respeitando a capacidade
    std::vector<int> genesCorrigidos = repararCVRP(filho);
    if(genesCorrigidos.empty()){
        return pai1;
    }
    return Individuo(genesCorrigidos, calcularFitness(genesCorrigidos));
}


Individuo GeneticAlgorithm::crossoverPMX(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal) {
    // 1. Extrair apenas os clientes (sem os 0s)
    std::vector<int> clientes1, clientes2;
    for (int g : pai1.genes) if (g != 0) clientes1.push_back(g);
    for (int g : pai2.genes) if (g != 0) clientes2.push_back(g);

    int n = clientes1.size();
    std::vector<int> filho(n, -1);

    // 2. Sorteia dois pontos de corte
    std::uniform_int_distribution<int> dist(0, n - 1);
    int c1 = dist(geradorLocal);
    int c2 = dist(geradorLocal);
    if (c1 > c2) std::swap(c1, c2);

    // 3. Copiar segmento do pai1 para o filho
    for (int i = c1; i <= c2; i++) {
        filho[i] = clientes1[i];
    }

    // 4. Mapear genes do pai2 → preencher conflitos
    for (int i = c1; i <= c2; i++) {
        int gene = clientes2[i];
        if (std::find(filho.begin(), filho.end(), gene) == filho.end()) {
            int pos = i;
            while (filho[pos] != -1) {
                int gene_p1 = clientes1[pos];
                pos = std::find(clientes2.begin(), clientes2.end(), gene_p1) - clientes2.begin();
            }
            filho[pos] = gene;
        }
    }

    // 5. Preencher os espaços vazios restantes com genes do pai2
    for (int i = 0; i < n; i++) {
        if (filho[i] == -1) {
            filho[i] = clientes2[i];
        }
    }

    // 6. Reconstruir cromossomo válido via reparador
    std::vector<int> cromossomo = repararCVRP(filho); 

    Individuo novo;
    novo.genes = cromossomo;
    novo.fitness = calcularFitness(cromossomo);

    return novo;
}

Individuo GeneticAlgorithm::crossoverCVRP(const Individuo &pai1, const Individuo &pai2, std::mt19937 &geradorLocal) {
    auto rotas_pai1 = extrairRotas(pai1.genes);
    auto rotas_pai2 = extrairRotas(pai2.genes);

    std::uniform_int_distribution<int> dist1(0, rotas_pai1.size() - 1);
    std::uniform_int_distribution<int> dist2(0, rotas_pai2.size() - 1);

    // Pega uma rota aleatória do pai 1
    std::vector<int> rota_herdada = rotas_pai1[dist1(geradorLocal)];
    
    std::unordered_set<int> clientes_servidos(rota_herdada.begin(), rota_herdada.end());
    
    // Constrói o filho
    Individuo filho;
    filho.genes.push_back(0);
    for(int cliente : rota_herdada) {
        filho.genes.push_back(cliente);
    }
    filho.genes.push_back(0);

    // Adiciona os clientes restantes do pai 2 que ainda não foram servidos
    std::vector<int> clientes_restantes;
    for (const auto& rota : rotas_pai2) {
        for (int cliente : rota) {
            if (clientes_servidos.find(cliente) == clientes_servidos.end()) {
                clientes_restantes.push_back(cliente);
            }
        }
    }
    
    // Usa a mesma lógica da geração de população para alocar os clientes restantes
    std::list<int> clientesNaoAtendidos(clientes_restantes.begin(), clientes_restantes.end());
    int veiculosUsados = 1; // Já usamos um veículo para a rota herdada

    while(!clientesNaoAtendidos.empty() && veiculosUsados < dataCVRP.numVeiculos) {
        veiculosUsados++;
        filho.genes.push_back(0);
        double cargaAtual = 0.0;
        
        for(auto it = clientesNaoAtendidos.begin(); it != clientesNaoAtendidos.end(); ){
            int clienteAtual = *it;
            if(cargaAtual + dataCVRP.demandas[clienteAtual] <= dataCVRP.capacidade){
                filho.genes.push_back(clienteAtual);
                cargaAtual += dataCVRP.demandas[clienteAtual];
                it = clientesNaoAtendidos.erase(it);
            } else {
                ++it;
            }
        }
        filho.genes.push_back(0);
    }

    // Se ainda sobraram clientes, o crossover falhou em criar uma solução válida.
    // Uma estratégia simples é retornar um dos pais.
    if (!clientesNaoAtendidos.empty()) {
        return pai1;
    }

    filho.fitness = calcularFitness(filho.genes); // Calcula o fitness com a função correta
    return filho;
}

// =====================
// Funções de migração (podem ser mantidas como estão)
// =====================
vectorIndiviudos GeneticAlgorithm::selecionarMigrantesUnicos(const vectorIndiviudos &populacao, std::mt19937 &gerador) {
    vectorIndiviudos selecionados;
    std::unordered_set<size_t> hashesVistos;

    std::vector<int> indices(populacao.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), gerador);

    for (int idx : indices) {
        const auto &ind = populacao[idx];
        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            hashesVistos.insert(h);
            selecionados.push_back(ind);
        }
        if (selecionados.size() >= tamMigracao) break;
    }
    return selecionados;
}

void GeneticAlgorithm::migracaoPopulacao(vectorIndiviudos &populacao, const vectorIndiviudos &selecionados) {
    int k = selecionados.size();
    if (k == 0 || populacao.empty()) return;

    if (k > populacao.size()) k = populacao.size();
    std::partial_sort(populacao.begin(), populacao.begin() + k, populacao.end(),
                      [](const Individuo &a, const Individuo &b){ return a.fitness > b.fitness; });

    for (int i = 0; i < k; ++i)
        populacao[i] = selecionados[i];
}


// =====================
// Funções de alto nível
// =====================

void GeneticAlgorithm::executarGeracao(Island &ilha, std::uniform_real_distribution<double> &distLocal) {
    if (ilha.populacao.empty()) return; // Guarda de segurança

    vectorIndiviudos elite = selecionarElite(ilha.populacao);
    int numFilhos = tamPopulacao - elite.size();
    vectorIndiviudos novaPop;
    novaPop.reserve(tamPopulacao);

    for (int i = 0; i < numFilhos; ++i) {
        auto pais = selecao(ilha.populacao, ilha.geradorlocal);
        // ALTERADO: Chamando os operadores corretos de CVRP
        Individuo prole = crossoverOX(pais[0], pais[1], ilha.geradorlocal);
        if (distLocal(ilha.geradorlocal) < probMutacao)
            mutacaoCVRP(prole, ilha.geradorlocal);
        novaPop.push_back(prole);
    }

    novaPop.insert(novaPop.end(), elite.begin(), elite.end());
    ilha.populacao = std::move(novaPop);
}

void GeneticAlgorithm::realizarMigracao(vetorIslands &islands) {
    for (auto &ilhaOrigem : islands) {
        auto selecionados = selecionarMigrantesUnicos(ilhaOrigem.populacao, ilhaOrigem.geradorlocal);
        for (int vizinhoId : ilhaOrigem.vizinhos) {
            auto it = std::find_if(islands.begin(), islands.end(),
                                     [vizinhoId](const Island &il){ return il.idIlha == vizinhoId; });
            if (it != islands.end())
                migracaoPopulacao(it->populacao, selecionados);
        }
    }
}

// =====================
// Função principal
// =====================

void GeneticAlgorithm::executarAlgoritmo() {
    std::uniform_real_distribution<double> distLocal(0, 1);
    Topologia topologia;
    auto islands = topologia.criarTopologia(TipoTopologia::Anel,numInslands,seed);

    std::cout << "Gerando população inicial..." << std::endl;
    // ALTERADO: Chamando a função de geração correta (agora renomeada)
    gerarPopulacao(islands);
    std::cout << "População inicial gerada." << std::endl;

    // ALTERADO: Descomentando o loop principal para executar o algoritmo
    Individuo melhor = melhorIndividuo(islands);
    std::cout << "Melhor fitness inicial: " << melhor.fitness << std::endl;

    for (int geracao = 0; geracao < numGeracoes; ++geracao) {
        for (auto &ilha : islands)
            executarGeracao(ilha, distLocal);

        if (geracao > 0 && geracao % freqMigracao == 0)
            realizarMigracao(islands);

        auto auxMelhor = melhorIndividuo(islands);
        if (auxMelhor.fitness < melhor.fitness) {
            melhor = auxMelhor;
            std::cout << "Geração " << geracao + 1 << " | Novo melhor fitness: " << melhor.fitness << std::endl;
        }
    }

    std::cout << "\n===================================" << std::endl;
    std::cout << "Evolução concluída!" << std::endl;
    std::cout << "Melhor fitness final: " << melhor.fitness << std::endl;
    std::cout << "Melhor solução encontrada: ";
    printVector(melhor.genes);
    std::cout << "===================================" << std::endl;
}