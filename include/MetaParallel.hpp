#pragma once
#include <vector>
#include <memory>
#include <string>
#include <chrono>

#include "AdvancedLocalSearch.hpp"
#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"


class MetaParallel {
public:
    DataLoader *data;
    int bestSolutionScore;
    int currentIterations;

    MetaParallel(DataLoader& data,
                 RandomSolver& randomSolver,
                 MemorySteepLocalSearch& localSearch,
                 unsigned int seed,
                 int maxIterations,
                 double timeLimit);

    void solve();
    std::string getAlgorithmName() const;
    void saveToFile(const std::string& filename = "") const;

private:
    std::vector<std::unique_ptr<RandomSolver>> randomSolvers_;
    std::vector<std::unique_ptr<MemorySteepLocalSearch>> localSearches_;
    std::vector<std::unique_ptr<AdvancedLocalSearch>> algorithms_;
    std::vector<int> bestSolution_;
};
