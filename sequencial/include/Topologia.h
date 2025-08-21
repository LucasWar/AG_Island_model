#include <vector>
#include <string>
#include "Individuo.h"
#include <random>
#include "Island.h"
#include "cvrpData.h"
enum class TipoTopologia {
    Malha,
    Anel
};


class Topologia{
private:
    std::vector<Island> criarMalha(int numIlhas, int seed);
    std::vector<Island> criarAnel(int numIlhas, int seed);
public:
    std::vector<Island> criarTopologia(TipoTopologia tipo,int numIslands, int seed);

};

