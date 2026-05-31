#pragma once
#include <random>
#include <string>
#include <vector>

#include "AdvancedLocalSearch.hpp"
#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"


class ILSv2 : public AdvancedLocalSearch {
public:
    std::mt19937 rng;

    ILSv2(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, int maxIterations = -1, double timeLimit = -1, bool needsStartingSolution = true, bool usesLocalSearch = true) : AdvancedLocalSearch(data, randomSolver, localSearch, maxIterations, timeLimit, needsStartingSolution, usesLocalSearch), rng(std::random_device{}()) {}
    ILSv2(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, unsigned int seed, int maxIterations = -1, double timeLimit = -1, bool needsStartingSolution = true, bool usesLocalSearch = true) : AdvancedLocalSearch(data, randomSolver, localSearch, maxIterations, timeLimit, needsStartingSolution, usesLocalSearch), rng(seed) {}
    std::string getAlgorithmName() const override;
    std::vector<int> createNewSolution() override;
private:
    int randomInt(int min, int max);
};
