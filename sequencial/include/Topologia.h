#include "Ilha.h"
#include "cvrpData.h"
#include "Individuo.h"
#include <vector>
#include <string>
#include <random>
enum class TipoTopologia {
    Malha,
    Anel
};


class Topologia{
private:
    std::vector<Ilha> criarMalha(int numIlhas, int seed);
    std::vector<Ilha> criarAnel(int numIlhas, int seed);
public:
    std::vector<Ilha> criarTopologia(std::string tipo,int numIlhas, int seed,std::string crossover,std::string selecao);

};

