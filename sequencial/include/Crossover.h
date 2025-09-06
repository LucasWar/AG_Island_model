#pragma once
#include "cvrpData.h"
#include "individuo.h"
#include <vector>
#include <string>
#include <random>

// Interface
class ICrossover {
public:
    virtual ~ICrossover() = default;
    virtual Individuo aplicar(const Individuo &pai1,
                                     const Individuo &pai2,
                                     std::mt19937 &geradorLocal,
                                     const CVRPData &dataCVRP) = 0;
};

// Implementação OX
class OXCrossover : public ICrossover {
public:
    Individuo aplicar(const Individuo &pai1,
                             const Individuo &pai2,
                             std::mt19937 &geradorLocal,
                             const CVRPData &dataCVRP) override;
};

// Implementação PMX
class PMXCrossover : public ICrossover {
public:
    Individuo aplicar(const Individuo &pai1,
                             const Individuo &pai2,
                             std::mt19937 &geradorLocal,
                             const CVRPData &dataCVRP) override;
};