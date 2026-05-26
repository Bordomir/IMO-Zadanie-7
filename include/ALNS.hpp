#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <random>
#include <utility>
#include <atomic>

#include "DataLoader.hpp"
#include "MemorySteepLocalSearch.hpp"

using namespace std;

class ALNS
{
public:
    DataLoader *data;

    vector<int> bestSolution;
    int bestSolutionScore;

    double destructionPercentage;

    atomic<int> currentIterations = 0;

    chrono::time_point<chrono::steady_clock> startTime;
    double timeLimit;

    vector<vector<double>> weights; 
    mutex weightsMutex;
    double weightDecay;
    double smartDestroyChance;

    double tempeatureStart;
    double temperatureEnd;

    ALNS(DataLoader& data, double destructionPercentage, double timeLimit, double weightDecay, double smartDestroyChance, double tempeatureStart, double temperatureEnd) : data(&data), destructionPercentage(destructionPercentage), timeLimit(timeLimit), weightDecay(weightDecay), smartDestroyChance(smartDestroyChance), tempeatureStart(tempeatureStart), temperatureEnd(temperatureEnd) {};

    void solve();

    void destroyHeuristic(vector<int>& solution, mt19937& rng) const;
    void destroySmart(vector<int>& solution, mt19937& rng);
    void repair(vector<int>& solution, mt19937& rng) const;
    pair<vector<int>,int> improveSolution(MemorySteepLocalSearch &LocalSearch, vector<int> &solution) const;

    void updateWeights(const vector<int>& solution, int scoreDifference);
    void evaporateWeights();

    int randomInt(int min, int max, mt19937& rng) const;

    string getAlgorithmName() const;
    int calculateScore(const vector<int> &solution) const;
    void print() const;
    void saveToFile(const string &filename = "") const;
};
