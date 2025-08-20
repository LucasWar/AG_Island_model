Comando para compilar
g++ -O2 -I include src/algoritmoGenetico.cpp src/individuo.cpp src/Utils.cpp src/main.cpp src/Island.cpp -o ag_exec

Executar
ag_exec

Paramentro a ser definidos na main
#define ARQUIVO_ENTRADA "entradas/kroA100.txt"
#define NUMERO_GERACOES 2000
#define PROB_MUTACAO    0.2
#define TAM_POPULCAO    120  => Tamanho da populacao
#define TAM_ELITE       0.1  => Tamanho da elite 
#define NUMISLANDS      2    => Numero de ilhas
