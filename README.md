# 🧬 Parallel Island Model Genetic Algorithm (CVRP Solver)

![C++](https://img.shields.io/badge/C++-17-blue.svg?style=for-the-badge&logo=c%2B%2B)
![OpenMP](https://img.shields.io/badge/OpenMP-Enabled-green.svg?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Completed-success.svg?style=for-the-badge)

Este projeto implementa um **Algoritmo Genético Paralelo baseado no Modelo de Ilhas (Island Model)** para resolver o **Problema de Roteamento de Veículos Capacitado (CVRP)**.

O objetivo principal foi superar as limitações dos modelos genéticos convencionais, utilizando paralelismo para:
1.  **Reduzir o tempo de execução** em instâncias grandes.
2.  **Melhorar a qualidade da solução**, evitando ótimos locais através da migração de indivíduos entre "ilhas" (populações isoladas).

---

## ⚙️ Características Técnicas

* **Linguagem:** C++17
* **Paralelismo:** OpenMP (para execução simultânea das ilhas).
* **Operadores Genéticos:**
    * *Crossover:* OX (Order Crossover) e PMX (Partially Mapped Crossover).
    * *Seleção:* Torneio e Roleta.
    * *Mutação:* Swap, Reversion, Insertion.
* **Estratégia:** Modelo de Ilhas com migração periódica de indivíduos e elitismo.

---

## 🚀 Como Executar

### Pré-requisitos
* Compilador C++ com suporte ao padrão C++17 (ex: GCC).
* Biblioteca **OpenMP** instalada.

### Compilação
Utilize o `g++` com a flag `-fopenmp` para habilitar o processamento paralelo:

```bash
g++ -std=c++17 -O2 -fopenmp \
-I include \
src/algoritmoGenetico.cpp src/CrossoverOXPMX.cpp src/Fitness.cpp \
src/individuo.cpp src/Ilha.cpp src/main.cpp src/Migracao.cpp \
src/Mutacao.cpp src/Reparar.cpp src/Selecao.cpp src/Populacao.cpp \
src/Topologia.cpp src/ReinicioPopulacao.cpp src/Utils.cpp \
-o ag_exec
