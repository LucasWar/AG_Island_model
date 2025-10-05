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


Evolução concluída!
Melhor fitness final: 1132
Melhor fitness possivel: 1053
Tempo de execução: 285.279
Melhor solução encontrada: [ 0, 106, 7, 123, 19, 49, 143, 36, 47, 124, 48, 82, 18, 0, 138, 12, 109, 150, 80, 68, 3, 77, 116, 76, 111, 0, 81, 135, 71, 103, 20, 128, 131, 32, 90, 126, 63, 64, 11, 107, 146, 0, 61, 113, 86, 140, 38, 14, 
119, 44, 141, 16, 91, 98, 0, 27, 69, 101, 70, 30, 122, 51, 9, 120, 33, 102, 50, 1, 132, 0, 53, 105, 26, 149, 40, 58, 137, 2, 115, 57, 144, 87, 117, 13, 0, 112, 89, 52, 88, 148, 62, 108, 10, 31, 127, 0, 118, 60, 83, 114, 8, 46, 45, 125, 17, 84, 5, 147, 0, 94, 95, 92, 37, 100, 85, 93, 59, 104, 99, 96, 6, 0, 97, 42, 142, 43, 15, 145, 41, 22, 133, 23, 56, 75, 74, 72, 73, 0, 21, 110, 4, 139, 39, 67, 25, 55, 130, 54, 134, 24, 29, 121, 0, 28, 79, 129, 78, 34, 35, 136, 
65, 66, 0 ]
Solução encontrada considerada mediana
GAP de 7.50237
Numero maximo de geracoes sem evolucao: 385
Solucão valida: 1