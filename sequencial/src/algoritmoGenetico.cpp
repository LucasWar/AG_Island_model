#include "algoritmoGenetico.h"
#include "Crossover.h"
#include "Topologia.h"
#include "cvrpData.h"
#include "Fitness.h"
#include "Utils.h"
#include "Reparar.h"
#include "Island.h"
#include <unordered_set>
#include <chrono>
#include <iostream>
#include <algorithm>
#include <list>

GeneticAlgorithm::GeneticAlgorithm(int nGen, float pMut, int tPop, float nElite, const std::string& caminho,std::uint64_t seed, int numInslands,float numMigracao,std::string opcTopologia,std::string opcCrossover,int freqMigracao) 
    :seed(seed),numGeracoes(nGen), probMutacao(pMut), tamPopulacao(tPop),numInslands(numInslands),opcTopologia(opcTopologia),opcCrossover(opcCrossover),freqMigracao(freqMigracao), tamMigracao(static_cast<int>(std::floor(numMigracao * tamPopulacao))), tamElite(static_cast<int>(std::floor(nElite * tamPopulacao))){
    dataCVRP = lerArquivoVRP(caminho);
    tamIndividuo = dataCVRP.distancias.empty() ? 0 : dataCVRP.distancias.size(); 


    if (opcCrossover == "OX")
        crossoverStrategy = std::make_unique<OXCrossover>();
    else if (opcCrossover == "PMX"){
        crossoverStrategy = std::make_unique<PMXCrossover>();
    }
    else
        throw std::invalid_argument("Tipo de crossover inválido: " + opcCrossover);
}

Individuo GeneticAlgorithm::gerarIndividuoUnicoDiversificado(std::mt19937 &gerador) {
    while (true) {
        std::vector<int> genes;
        std::uniform_int_distribution<int> dist_operador(1, 100);
        int escolha = dist_operador(gerador);

        if (escolha <= 40) { // 40% com Aleatorio-Gulos
            genes = criarIndividuo_AleatorioGulos(gerador);
        } else if (escolha <= 80) { // 40% com Vizinho Mais Próximo
            genes = criarIndividuo_VizinhoMaisProximo(gerador);
        } else { // 20% com Inserção Aleatória
            genes = criarIndividuo_InsercaoAleatoria(gerador);
        }

        if (!genes.empty()) {
            return Individuo(genes, calcularFitness(genes, dataCVRP));
        }
    }
}


void GeneticAlgorithm::reiniciarPopulacoes(Island &ilha, int geracaoAtual) {
    std::cout << "\n*** ESTAGNAÇÃO DETECTADA! REINICIANDO POPULAÇÕES PARCIALMENTE. Ilha: " << ilha.idIlha << ", Geração: " << geracaoAtual << " ***\n" << std::endl;
    
    if (ilha.populacao.size() < 10) return; // Evita problemas com populações muito pequenas

    const double percentualMelhores = 0.01; // 5% de elite
    const double percentualAleatorios = 0.15; // 10% de "sobreviventes aleatórios"

    size_t numMelhoresAPreservar = static_cast<size_t>(std::floor(tamPopulacao * percentualMelhores));
    size_t numAleatoriosAPreservar = static_cast<size_t>(std::floor(tamPopulacao * percentualAleatorios));
    
    if (numMelhoresAPreservar + numAleatoriosAPreservar >= tamPopulacao) {
        numAleatoriosAPreservar = tamPopulacao - numMelhoresAPreservar;
    }
    size_t numAGerar = tamPopulacao - numMelhoresAPreservar - numAleatoriosAPreservar;

    vectorIndiviudos novaPopulacao;
    novaPopulacao.reserve(tamPopulacao);
    std::unordered_set<size_t> hashesVistos; // Para garantir a unicidade de TODOS os preservados

    std::sort(ilha.populacao.begin(), ilha.populacao.end(), 
              [](const Individuo& a, const Individuo& b) {
                  return a.fitness < b.fitness;
              });

    for (const auto& ind : ilha.populacao) {
        if (novaPopulacao.size() >= numMelhoresAPreservar) break;

        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            novaPopulacao.push_back(ind);
            hashesVistos.insert(h);
        }
    }

    std::vector<int> indicesCandidatos;
    for (size_t i = 0; i < ilha.populacao.size(); ++i) {
        size_t h = hashGenes(ilha.populacao[i].genes);
        if (hashesVistos.find(h) == hashesVistos.end()) {
            indicesCandidatos.push_back(i);
        }
    }

    std::shuffle(indicesCandidatos.begin(), indicesCandidatos.end(), ilha.geradorlocal);

    size_t aleatoriosAdicionados = 0;
    for (int idx : indicesCandidatos) {
        if (aleatoriosAdicionados >= numAleatoriosAPreservar) break;
        
        const auto& ind = ilha.populacao[idx];
        size_t h = hashGenes(ind.genes);
        if (hashesVistos.find(h) == hashesVistos.end()){
             novaPopulacao.push_back(ind);
             hashesVistos.insert(h);
             aleatoriosAdicionados++;
        }
    }
    
    numAGerar = tamPopulacao - novaPopulacao.size(); 
    for (size_t i = 0; i < numAGerar; ++i) {
        novaPopulacao.push_back(gerarIndividuoUnicoDiversificado(ilha.geradorlocal));
    }

    ilha.populacao = std::move(novaPopulacao);

    auto it = std::min_element(ilha.populacao.begin(), ilha.populacao.end(),
                              [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

    if (it != ilha.populacao.end()) {
        ilha.melhorIndividuoIlhaa = *it;
        ilha.geracaoUltimaEvolucao = geracaoAtual; 
    }
}

Individuo GeneticAlgorithm::melhorIndividuo(vetorIslands &islands, int geracao) {
    bool inicializado = false;
    Individuo melhor;
    for (auto &island : islands) {
        if (island.populacao.empty()) continue;

        auto it = std::min_element(island.populacao.begin(), island.populacao.end(),
                                     [](const Individuo& a, const Individuo& b) { return a.fitness < b.fitness; });

        if(it->fitness < island.melhorIndividuoIlhaa.fitness){
            island.melhorIndividuoIlhaa = *it;
            island.geracaoUltimaEvolucao = geracao;
        }else if(geracao - island.geracaoUltimaEvolucao >= island.resetNum){
            island.resetNum += 100;
            island.reset = true;
            std::cout << island.resetNum << std::endl;
        }

        if (!inicializado || it->fitness < melhor.fitness) {
            melhor = *it;
            inicializado = true;
        }
        
    }
    
    if (!inicializado) throw std::runtime_error("Nenhuma população encontrada para avaliar.");
    return melhor;
}

void GeneticAlgorithm::executarAlgoritmo() {
    std::cout << "Numero de ilhas: " << numInslands << std::endl;
    std::cout << "Crossover selecionado: " << opcCrossover << std::endl;
    std::cout << "Topologia Selecionada: "<< opcTopologia << std::endl;
    std::cout << "Numero de individuos por ilha: "<< tamPopulacao << std::endl;
    std::cout << "Numero de individuos para migração: "<< tamMigracao << std::endl;
    std::cout << "Frequencia de migração: "<< freqMigracao << std::endl;
    std::cout << "Preservação de elite: "<< tamElite << std::endl;

    int numGerSemEvo = 0;
    int auxNumGerSemEvo = 0;
    int contadorReinicializacoes = 0; // NOVO: Para contar os restarts
    std::uniform_real_distribution<double> distLocal(0, 1);
    Topologia topologia;
    auto islands = topologia.criarTopologia(opcTopologia,numInslands,seed);
    auto inicio = std::chrono::high_resolution_clock::now();
    std::cout << "Gerando população inicial..." << std::endl;
    gerarPopulacaoDiversificada(islands);
    std::cout << "População inicial gerada." << std::endl;

    Individuo melhor = melhorIndividuo(islands,0);
    std::cout << "Melhor fitness inicial: " << melhor.fitness << std::endl;

    for (int geracao = 0; geracao < numGeracoes; ++geracao) {

        for (auto &ilha : islands)
            if(ilha.reset == true){
                reiniciarPopulacoes(ilha,geracao);
                ilha.reset = false;
            }
        if(contadorReinicializacoes > 8){
            break;
        }
        for (auto &ilha : islands)
            executarGeracao(ilha, distLocal);

        if (geracao > 0 and (geracao % freqMigracao) == 0)   {
            realizarMigracao_Aprimorada(islands);
        }
            
        auto auxMelhor = melhorIndividuo(islands, geracao);
        
        if (auxMelhor.fitness < melhor.fitness) {
            melhor = auxMelhor;
            if (auxNumGerSemEvo > numGerSemEvo) {
                numGerSemEvo = auxNumGerSemEvo;
            }
            auxNumGerSemEvo = 0;
            std::cout << "Geração " << geracao + 1 << " | Novo melhor fitness: " << melhor.fitness << std::endl;
        } else {
            auxNumGerSemEvo++;
        }

    }
    auto fim = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duracao = fim - inicio;

    std::cout << "\n===================================" << std::endl;
    std::cout << "Evolução concluída!" << std::endl;
    std::cout << "Melhor fitness final: " << melhor.fitness << std::endl;
    std::cout << "Melhor fitness possivel: " << dataCVRP.solucaoOtima << std::endl;
    std::cout << "Tempo de execução: " << duracao.count() << std::endl;
    std::cout << "Melhor solução encontrada: ";
    printVector(melhor.genes);
    double classificar = ((double)(melhor.fitness - dataCVRP.solucaoOtima)/dataCVRP.solucaoOtima * 100 );
    if(classificar >= 0 and classificar <= 5){
        std::cout << "Solução encontrada considerada boa"<< std::endl;
    } else if(classificar > 5 and classificar <= 8){
        std::cout << "Solução encontrada considerada mediana"<< std::endl;
    }
    else{
        std::cout << "Solução encontrada considerada ruim"<< std::endl;
    }
    std::cout << "GAP de "<< classificar << std::endl;
    std::cout << "Numero maximo de geracoes sem evolucao: " << numGerSemEvo << std::endl;
    
    std::cout << "===================================" << std::endl;
}