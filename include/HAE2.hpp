#pragma once

#include <string>
#include <vector>

#include "EvolutionMetaheuristic.hpp"
#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"

using namespace std;

class HAE2 : public EvolutionMetaheuristic {
public:
    HAE2(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch,int eliteSize = 20, int maxIterations = -1, double timeLimit = -1, bool usesLocalSearch = true, unsigned int seed = 0) : EvolutionMetaheuristic(data, randomSolver, localSearch, eliteSize, maxIterations, timeLimit, usesLocalSearch, seed) {}
    string getAlgorithmName() const override;
    vector<int> recombineParents(vector<int> &firstParent, vector<int> &secondParent) override;
};