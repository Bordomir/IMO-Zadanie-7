#pragma once

#include <random>

#include "Solver.hpp"
#include "DataLoader.hpp"

class RandomSolver : public Solver
{
public:
    std::mt19937 rng;

    RandomSolver(DataLoader &data) : Solver(data, 0), rng(std::random_device{}()) {};
    RandomSolver(DataLoader &data, unsigned int seed) : Solver(data, 0), rng(seed) {};
    RandomSolver(DataLoader &data, mt19937 &rng) : Solver(data, 0), rng(rng) {};
    string getAlgorithmName() override;
    void solve() override;

private:
    int randomInt(int min, int max);
};