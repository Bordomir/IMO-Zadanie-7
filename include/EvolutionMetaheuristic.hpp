#pragma once

#include <vector>
#include <chrono>
#include <string>
#include <random>
#include <utility>

#include "DataLoader.hpp"
#include "RandomSolver.hpp"
#include "MemorySteepLocalSearch.hpp"

using namespace std;

class EvolutionMetaheuristic
{
public:
    DataLoader *data;
    RandomSolver *randomSolver;
    MemorySteepLocalSearch *LocalSearch;

    std::mt19937 rng;

    int eliteSize;

    vector<vector<int>> bestSolutions;
    vector<int> bestSolutionScores;

    int currentIterations;
    chrono::time_point<chrono::steady_clock> startTime;

    // Stop conditions
    bool usesTimeLimit;
    int maxIterations;
    double timeLimit;

    // Flags
    bool usesLocalSearch;
    
    EvolutionMetaheuristic(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch,int eliteSize = 20, int maxIterations = -1, double timeLimit = -1, bool usesLocalSearch = true, unsigned int seed = 0);
    virtual ~EvolutionMetaheuristic() = default;
    virtual string getAlgorithmName() const = 0;
    void setData(DataLoader &data);
    void solve();
    vector<int> getRandomizedSolution() const;
    vector<int> improveSolution(vector<int> &solution) const;
    bool canContinue() const;
    bool isSolutionDifferent(int solutionScore) const;
    bool isSolutionDifferent(vector<int> &solution) const;
    void repair(vector<int> &solution);
    vector<pair<int, int>> makeHelperArray(vector<int> &solution) const;
    vector<int> makeInSolutionArray(vector<int> &solution) const;
    virtual vector<int> recombineParents(vector<int> &firstParent, vector<int> &secondParent) = 0;
    int calculateScore(const vector<int> &solution) const;
    int getBestSolutionIndex() const;
    void print() const;
    void saveToFile(const string &filename = "") const;
private:
    int randomInt(int min, int max);
};
