#include "utils.h"
#include "Ilha.h"
#include "cvrpData.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "cvrpData.h"

#define TRUE
#define FALSE

namespace fs = std::filesystem;

std::vector<std::vector<double>> lerArquivo(const std::string& caminhoArquivo) {
    std::ifstream arquivo(caminhoArquivo);
    std::vector<std::vector<double>> matriz;
    std::string linha;
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo: " << caminhoArquivo << std::endl;
        return {};
    }
    while (std::getline(arquivo, linha)) {
        std::istringstream iss(linha);
        std::vector<double> linhaNumeros;
        double num;
        while (iss >> num) linhaNumeros.push_back(num);
        if (!linhaNumeros.empty()) matriz.push_back(std::move(linhaNumeros));
    }
    return matriz;
}


std::uint64_t lerArquivoSeed(const std::string& nomeArquivo, std::size_t inicio, std::size_t fim) {
    fs::path caminhoExecutavel = fs::current_path();
    fs::path caminhoArquivo = caminhoExecutavel / nomeArquivo;

    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Erro ao abrir o arquivo: " + caminhoArquivo.string());
    }

    std::string linhaCompleta;
    std::getline(arquivo, linhaCompleta);
    arquivo.close();

    if (fim > linhaCompleta.size() || inicio >= fim) {
        throw std::invalid_argument("Intervalo inválido de leitura.");
    }

    std::string trecho = linhaCompleta.substr(inicio, fim - inicio);
    trecho.erase(0, trecho.find_first_not_of('0'));
    if (trecho.empty()) trecho = "0";

    std::uint64_t resultado = 0;
    for (char c : trecho) {
        if (c < '0' || c > '9') {
            throw std::invalid_argument("Caractere inválido encontrado.");
        }
        resultado = resultado * 10 + (c - '0');
    }

    return resultado;
}

double distanciaEuclidiana(double x1, double y1, double x2, double y2) {
    double dx = x1 - x2;
    double dy = y1 - y2;
    return std::sqrt(dx * dx + dy * dy);
}

CVRPData lerArquivoVRP(const std::string& caminhoArquivo) {
    std::ifstream arquivo(caminhoArquivo);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Erro ao abrir arquivo: " + caminhoArquivo);
    }

    std::string linha;
    int dimension = 0;
    int capacidade = 0;
    int numVeiculos = 1;
    int solucaoOtima = 0;
    std::string nome;
    std::vector<std::pair<double, double>> coordenadas;
    std::vector<int> demandas;
    int deposito = -1;

    // Flags para identificar em qual seção estamos
    bool emNodeCoord = false, emDemand = false, emDepot = false;

    while (std::getline(arquivo, linha)) {
        if (linha.find("DIMENSION") != std::string::npos) {
            dimension = std::stoi(linha.substr(linha.find(":") + 1));
        }
        else if (linha.find("TRUCKS") != std::string::npos) {
            numVeiculos = std::stoi(linha.substr(linha.find(":") + 1));
        }
        else if (linha.find("OPTIMAL") != std::string::npos) {
            solucaoOtima = std::stoi(linha.substr(linha.find(":") + 1));
        }
        else if (linha.find("CAPACITY") != std::string::npos) {
            capacidade = std::stoi(linha.substr(linha.find(":") + 1));
        }
        else if (linha.find("NAME") != std::string::npos) {
            nome = linha.substr(linha.find(":") + 1);
        }
        else if (linha.find("NODE_COORD_SECTION") != std::string::npos) {
            emNodeCoord = true;
            continue;
        }
        else if (linha.find("DEMAND_SECTION") != std::string::npos) {
            emNodeCoord = false;
            emDemand = true;
            continue;
        }
        else if (linha.find("DEPOT_SECTION") != std::string::npos) {
            emDemand = false;
            emDepot = true;
            continue;
        }
        else if (linha.find("EOF") != std::string::npos) {
            break;
        }

        if (emNodeCoord) {
            std::istringstream iss(linha);
            int id;
            double x, y;
            if (iss >> id >> x >> y) {
                if ((int)coordenadas.size() < dimension)
                    coordenadas.push_back({x, y});
            }
        }
        else if (emDemand) {
            std::istringstream iss(linha);
            int id, d;
            if (iss >> id >> d) {
                if ((int)demandas.size() < dimension)
                    demandas.push_back(d);
            }
        }
        else if (emDepot) {
            int id = std::stoi(linha);
            if (id == -1) {
                emDepot = false; // fim da seção
            } else {
                deposito = id - 1; // indexando a partir de 0
            }
        }
    }

    // Construir a matriz de distâncias
    std::vector<std::vector<double>> distancias(dimension, std::vector<double>(dimension, 0.0));
    for (int i = 0; i < dimension; i++) {
        for (int j = 0; j < dimension; j++) {
            if (i != j) {
                distancias[i][j] = distanciaEuclidiana(coordenadas[i].first, coordenadas[i].second,
                                                      coordenadas[j].first, coordenadas[j].second);
            }
        }
    }

    CVRPData dados{distancias, demandas, nome, capacidade, deposito, numVeiculos, solucaoOtima};
    return dados;
}

void printVector(const std::vector<int>& vec, const std::string& label) {
    if (!label.empty()) std::cout << label << ": ";
    std::cout << "[ ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << vec[i];
        if (i < vec.size() - 1) std::cout << ", ";
    }
    std::cout << " ]\n";
}


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
            if (gene < 0 || gene >= dataCVRP.distancias.size()) return 0; // cliente inválido
            if (clientesAtendidos[gene]) return 0; // cliente duplicado
            clientesAtendidos[gene] = true;

            cargaAtual += dataCVRP.demandas[gene];
            if (cargaAtual > dataCVRP.capacidade) return 0; // capacidade estourada
        }
    }

    // Número de veículos não pode ultrapassar o limite
    if (veiculosUsados > dataCVRP.numVeiculos) return 0;

    // Verifica se todos os clientes foram atendidos
    for (size_t c = 1; c < dataCVRP.distancias.size(); ++c) { // 0 = depósito
        if (!clientesAtendidos[c]) return 0;
    }

    return 1; // passou em todas as verificações
}


size_t hashGenes(const std::vector<int>& genes) {
    size_t seed = genes.size();
    for (int g : genes) {
        seed ^= std::hash<int>()(g) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
    return seed;
}

void salvarResultados(const Individuo& melhor, int solucaoOtima, int duracao, int numGerSemEvo, const std::vector<int>& genes, const CVRPData& dataCVRP, int numIlhas, std::string opcTopologia) {
    // Abrir o arquivo no modo append para não sobrescrever os dados
    std::ofstream arquivo("resultados_"+dataCVRP.nome+"_Ilhas_" + std::to_string(numIlhas) + "_" + opcTopologia + "_" +".txt", std::ios::app); // O parâmetro std::ios::app garante que o arquivo será atualizado.

    // Verificar se o arquivo foi aberto corretamente
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao abrir o arquivo para escrita!" << std::endl;
        return;
    }

    // Escrever os resultados no arquivo
    arquivo << "\n ===================================================RESULTADOS=========================================================\n";
    arquivo << "Melhor fitness final: " << melhor.fitness << "\n";
    arquivo << "Melhor fitness possivel: " << solucaoOtima << "\n";
    arquivo << "Tempo de execução: " << duracao << "\n";
    
    // Função para imprimir o vetor no arquivo
    arquivo << "Melhor solução encontrada: ";
    for (const auto& gene : genes) {
        arquivo << gene << " ";
    }
    arquivo << "\n";
    
    double classificar = ((double)(melhor.fitness - solucaoOtima) / solucaoOtima) * 100;
    
    if (classificar >= 0 && classificar <= 5) {
        arquivo << "Solução encontrada considerada boa\n";
    } else if (classificar > 5 && classificar <= 8) {
        arquivo << "Solução encontrada considerada mediana\n";
    } else {
        arquivo << "Solução encontrada considerada ruim\n";
    }

    arquivo << "GAP de " << classificar << "\n";
    arquivo << "Numero maximo de geracoes sem evolucao: " << numGerSemEvo << "\n";
    
    // Supondo que 'verificarValidadeCVRP' seja uma função que retorna um valor booleano
    bool solucaoValida = verificarValidadeCVRP(genes, dataCVRP); 
    arquivo << "Solucão valida: " << (solucaoValida ? "Sim" : "Não") << "\n";
    
    arquivo << "====================================================================================================================\n\n";

    // Fechar o arquivo
    arquivo.close();
}


std::vector<int> extrairClientes(const std::vector<int>& genes) {
    std::vector<int> clientes;
    clientes.reserve(genes.size());
    for (int g : genes) {
        if (g != 0) {
            clientes.push_back(g);
        }
    }
    return clientes;
}

std::vector<std::vector<int>> extrairRotas(const std::vector<int>& genes) {
    std::vector<std::vector<int>> rotas;
    std::vector<int> rotaAtual;

    for (int g : genes) {
        if (g == 0) {
            if (!rotaAtual.empty()) {
                rotas.push_back(rotaAtual);
                rotaAtual.clear();
            }
        } else {
            rotaAtual.push_back(g);
        }
    }

    // Adiciona última rota se não terminar com zero
    if (!rotaAtual.empty()) {
        rotas.push_back(rotaAtual);
    }

    return rotas;
}

std::pair<int, int> sortearPontosCorte(int tamanho, std::mt19937& gerador) {
    std::uniform_int_distribution<int> dist(0, tamanho - 1);
    int c1 = dist(gerador);
    int c2 = dist(gerador);
    if (c1 > c2) std::swap(c1, c2);
    return {c1, c2};
}