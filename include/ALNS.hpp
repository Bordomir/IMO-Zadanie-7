#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <mutex>
#include <random>

#include "DataLoader.hpp"

using namespace std;

class ALNS
{
public:
    DataLoader *data;

    vector<int> bestSolution;
    int bestSolutionScore;

    double destructionPercentage;

    chrono::time_point<chrono::steady_clock> startTime;
    double timeLimit;

    vector<vector<double>> weights; 
    mutex weightsMutex;
    double weightDecay;

    double tempeatureStart;
    double temperatureEnd;
    double coolingRate;

    ALNS(DataLoader& data, double destructionPercentage, double timeLimit, double weightDecay, double tempeatureStart, double temperatureEnd, double coolingRate) : data(&data), destructionPercentage(destructionPercentage), timeLimit(timeLimit), weightDecay(weightDecay), tempeatureStart(tempeatureStart), temperatureEnd(temperatureEnd), coolingRate(coolingRate) {};

    void solve();

    void destroyHeuristic(vector<int>& solution, mt19937& rng);
    void destroySmart(vector<int>& solution, mt19937& rng);
    void repair(vector<int>& solution, mt19937& rng);

    int randomInt(int min, int max, mt19937& rng);

    string getAlgorithmName() const;
    int calculateScore(const vector<int> &solution) const;
    void print() const;
    void saveToFile(const string &filename = "") const;
};
