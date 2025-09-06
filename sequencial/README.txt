Comando para compilar
g++ -std=c++17 -O2 -I include src/algoritmoGenetico.cpp src/CrossoverOXPMX.cpp src/Fitness.cpp src/individuo.cpp src/Island.cpp src/main.cpp src/Migracao.cpp src/Mutacao.cpp src/Reparar.cpp src/Selecao.cpp src/Populacao.cpp src/Topologia.cpp src/Utils.cpp -o ag_exec

Executar
ag_exec

Paramentro a ser definidos na main
#define ARQUIVO_ENTRADA "entradas/kroA100.txt"
#define NUMERO_GERACOES 2000
#define PROB_MUTACAO    0.2
#define TAM_POPULCAO    120  => Tamanho da populacao
#define TAM_ELITE       0.1  => Tamanho da elite 
#define NUMISLANDS      2    => Numero de ilhas
