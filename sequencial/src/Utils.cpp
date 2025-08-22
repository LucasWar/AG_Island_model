#include "utils.h"
#include "Island.h"
#include "cvrpData.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

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
        else if (linha.find("CAPACITY") != std::string::npos) {
            capacidade = std::stoi(linha.substr(linha.find(":") + 1));
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

    CVRPData dados{distancias, demandas, capacidade, deposito, numVeiculos};
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