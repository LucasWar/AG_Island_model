#include "Island.h"
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
    std::vector<Island> criarMalha(int numIlhas, int seed);
    std::vector<Island> criarAnel(int numIlhas, int seed);
public:
    std::vector<Island> criarTopologia(std::string tipo,int numIslands, int seed);

};

