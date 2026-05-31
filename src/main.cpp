#include <string>
#include <vector>
#include <memory>
#include <print>
#include <random>
#include <limits>
#include <ranges>
#include <algorithm>
#include <utility>
#include <chrono>

#include "../include/DataLoader.hpp"
#include "../include/Solver.hpp"
#include "../include/RandomSolver.hpp"
#include "../include/KRegret.hpp"
#include "../include/LocalSearch.hpp"
#include "../include/MemorySteepLocalSearch.hpp"
#include "../include/AdvancedLocalSearch.hpp"
#include "../include/MSLS.hpp"
#include "../include/ILS.hpp"
#include "../include/LNS.hpp"
#include "../include/EvolutionMetaheuristic.hpp"
#include "../include/HAE1.hpp"
#include "../include/HAE2.hpp"
#include "../include/HAE3.hpp"
#include "../include/ALNS.hpp"
#include "../include/MetaParallel.hpp"

using namespace std;

struct Statistic
{
    string data;
    string solver;
    double average = 0;
    double min = numeric_limits<double>::max();
    double max = numeric_limits<double>::min();

    Statistic(string data, string solver) : data(move(data)), solver(move(solver)) {};
    void update(double value)
    {
        average += value;
        min = std::min(min, value);
        max = std::max(max, value);
    }
    void print() const
    {
        std::println("{};{};{:.4f};{:.4f};{:.4f}", data, solver, average, min, max);
    }
};

int main()
{
    DataLoader dataA("../data/TSPA.csv", "DataA");
    DataLoader dataB("../data/TSPB.csv", "DataB");

    RandomSolver randomSolverA(dataA, 0);
    RandomSolver randomSolverB(dataB, 0);

    MemorySteepLocalSearch LocalSearchA(dataA, vector<int>{});
    MemorySteepLocalSearch LocalSearchB(dataB, vector<int>{});

    vector<unique_ptr<AdvancedLocalSearch>> MSLSadvancedLocalSearches;
    MSLSadvancedLocalSearches.reserve(2);
    MSLSadvancedLocalSearches.emplace_back(make_unique<MSLS>(dataA, randomSolverA, LocalSearchA, 200, -1, false, true));
    MSLSadvancedLocalSearches.emplace_back(make_unique<MSLS>(dataB, randomSolverB, LocalSearchB, 200, -1, false, true));

    // Experiment
    int numRuns = 20;
    chrono::time_point<chrono::steady_clock> startTime, endTime;

    vector<Statistic> MSLSscoreStatistics;
    MSLSscoreStatistics.reserve(MSLSadvancedLocalSearches.size());
    vector<Statistic> MSLStimeStatistics;
    MSLStimeStatistics.reserve(MSLSadvancedLocalSearches.size());
    vector<Statistic> MSLSiterationStatistics;
    MSLSiterationStatistics.reserve(MSLSadvancedLocalSearches.size());

    for (auto &solver : MSLSadvancedLocalSearches)
    {
        MSLSscoreStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        MSLStimeStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        MSLSiterationStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < MSLSadvancedLocalSearches.size(); i++)
        {
            const auto &solver = MSLSadvancedLocalSearches[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();

            if (solver->bestSolutionScore > MSLSscoreStatistics[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            MSLSscoreStatistics[i].update(solver->bestSolutionScore);
            MSLStimeStatistics[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
            MSLSiterationStatistics[i].update(solver->currentIterations);
        }
    }
    for (auto &stat : MSLSscoreStatistics)
        stat.average /= numRuns;
    for (auto &stat : MSLStimeStatistics)
        stat.average /= numRuns;
    for (auto &stat : MSLSiterationStatistics)
        stat.average /= numRuns;

    double timeLimitA = MSLStimeStatistics[0].average;
    double timeLimitB = MSLStimeStatistics[1].average;
    println("Time limits: {:.4f}; {:.4f}", timeLimitA, timeLimitB);

    // double timeLimitA = 4329.2627;
    // double timeLimitB = 4687.7395;
    // double timeLimitA = 100.0;
    // double timeLimitB = 100.0;

    vector<unique_ptr<AdvancedLocalSearch>> advancedLocalSearches;
    advancedLocalSearches.reserve(6);
    constexpr unsigned int SEED = 0;
    advancedLocalSearches.emplace_back(make_unique<ILS>(dataA, randomSolverA, LocalSearchA, SEED, -1, timeLimitA, true, true));
    advancedLocalSearches.emplace_back(make_unique<ILS>(dataB, randomSolverB, LocalSearchB, SEED, -1, timeLimitB, true, true));
    // advancedLocalSearches.emplace_back(make_unique<LNS>(dataA, randomSolverA, LocalSearchA, SEED, 30, -1, timeLimitA, true, false));
    // advancedLocalSearches.emplace_back(make_unique<LNS>(dataB, randomSolverB, LocalSearchB, SEED, 30, -1, timeLimitB, true, false));
    // advancedLocalSearches.emplace_back(make_unique<LNS>(dataA, randomSolverA, LocalSearchA, SEED, 30, -1, timeLimitA, true, true));
    // advancedLocalSearches.emplace_back(make_unique<LNS>(dataB, randomSolverB, LocalSearchB, SEED, 30, -1, timeLimitB, true, true));

    vector<Statistic> scoreStatistics;
    scoreStatistics.reserve(advancedLocalSearches.size());
    vector<Statistic> timeStatistics;
    timeStatistics.reserve(advancedLocalSearches.size());
    vector<Statistic> iterationStatistics;
    iterationStatistics.reserve(advancedLocalSearches.size());
    for (auto &solver : advancedLocalSearches)
    {
        scoreStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        timeStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        iterationStatistics.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < advancedLocalSearches.size(); i++)
        {
            const auto &solver = advancedLocalSearches[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();

            if (solver->bestSolutionScore > scoreStatistics[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            scoreStatistics[i].update(solver->bestSolutionScore);
            timeStatistics[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
            iterationStatistics[i].update(solver->currentIterations);
        }
    }
    for (auto &stat : scoreStatistics)
        stat.average /= numRuns;
    for (auto &stat : timeStatistics)
        stat.average /= numRuns;
    for (auto &stat : iterationStatistics)
        stat.average /= numRuns;

    vector<unique_ptr<EvolutionMetaheuristic>> evolutionSolvers;
    evolutionSolvers.reserve(4);
    evolutionSolvers.emplace_back(make_unique<HAE1>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, true, SEED));
    evolutionSolvers.emplace_back(make_unique<HAE1>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, true, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE2>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, true, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE2>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, true, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE2>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, false, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE2>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, false, SEED));
    // // evolutionSolvers.emplace_back(make_unique<HAE3>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, true, SEED));
    // // evolutionSolvers.emplace_back(make_unique<HAE3>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, true, SEED));
    // // evolutionSolvers.emplace_back(make_unique<HAE3>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, false, SEED));
    // // evolutionSolvers.emplace_back(make_unique<HAE3>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, false, SEED));
    
    vector<Statistic> scoreStatisticsEvolution;
    scoreStatisticsEvolution.reserve(evolutionSolvers.size());
    vector<Statistic> timeStatisticsEvolution;
    timeStatisticsEvolution.reserve(evolutionSolvers.size());
    vector<Statistic> iterationStatisticsEvolution;
    iterationStatisticsEvolution.reserve(evolutionSolvers.size());
    for (auto &solver : evolutionSolvers)
    {
        scoreStatisticsEvolution.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        timeStatisticsEvolution.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        iterationStatisticsEvolution.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < evolutionSolvers.size(); i++)
        {
            const auto &solver = evolutionSolvers[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();
            
            int bestSolutionIndex = solver->getBestSolutionIndex();
            vector<int> bestSolution = solver->bestSolutions[bestSolutionIndex];
            int bestSolutionScore = solver->bestSolutionScores[bestSolutionIndex];

            if (bestSolutionScore > scoreStatisticsEvolution[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            scoreStatisticsEvolution[i].update(bestSolutionScore);
            timeStatisticsEvolution[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
            iterationStatisticsEvolution[i].update(solver->currentIterations);
        }
    }
    for (auto &stat : scoreStatisticsEvolution)
        stat.average /= numRuns;
    for (auto &stat : timeStatisticsEvolution)
        stat.average /= numRuns;
    for (auto &stat : iterationStatisticsEvolution)
        stat.average /= numRuns;
    
    vector<unique_ptr<ALNS>> appliedLNSsolvers;
    appliedLNSsolvers.reserve(8);
    // evolutionSolvers.emplace_back(make_unique<HAE1>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, true, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE1>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, true, SEED));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataA, 0.65, timeLimitA, 0.9, 0.75, 1.00, 0.01));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataA, 0.65, timeLimitA, 0.9, 0.75, 0.75, 0.01));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataA, 0.65, timeLimitA, 0.9, 0.75, 0.50, 0.01));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataA, 0.65, timeLimitA, 0.9, 0.75, 0.25, 0.01));

    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataB, 0.65, timeLimitB, 0.9, 0.75, 1.00, 0.01));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataB, 0.65, timeLimitB, 0.9, 0.75, 0.75, 0.01));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataB, 0.65, timeLimitB, 0.9, 0.75, 0.50, 0.01));
    appliedLNSsolvers.emplace_back(make_unique<ALNS>(dataB, 0.65, timeLimitB, 0.9, 0.75, 0.25, 0.01));
    // evolutionSolvers.emplace_back(make_unique<HAE3>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, true, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE3>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, true, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE3>(dataA, randomSolverA, LocalSearchA, 20, -1, timeLimitA, false, SEED));
    // evolutionSolvers.emplace_back(make_unique<HAE3>(dataB, randomSolverB, LocalSearchB, 20, -1, timeLimitB, false, SEED));
    
    vector<Statistic> scoreStatisticsALNS;
    scoreStatisticsALNS.reserve(appliedLNSsolvers.size());
    vector<Statistic> timeStatisticsALNS;
    timeStatisticsALNS.reserve(appliedLNSsolvers.size());
    vector<Statistic> iterationStatisticsALNS;
    iterationStatisticsALNS.reserve(appliedLNSsolvers.size());
    for (auto &solver : appliedLNSsolvers)
    {
        scoreStatisticsALNS.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        timeStatisticsALNS.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        iterationStatisticsALNS.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < appliedLNSsolvers.size(); i++)
        {
            auto &solver = appliedLNSsolvers[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();
            
            vector<int> bestSolution = solver->bestSolution;
            int bestSolutionScore = solver->bestSolutionScore;

            if (bestSolutionScore > scoreStatisticsALNS[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            scoreStatisticsALNS[i].update(bestSolutionScore);
            timeStatisticsALNS[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
            iterationStatisticsALNS[i].update(solver->currentIterations);
        }
    }
    for (auto &stat : scoreStatisticsALNS)
        stat.average /= numRuns;
    for (auto &stat : timeStatisticsALNS)
        stat.average /= numRuns;
    for (auto &stat : iterationStatisticsALNS)
        stat.average /= numRuns;
    
    vector<unique_ptr<MetaParallel>> metaParallelSolvers;
    metaParallelSolvers.reserve(2);
    metaParallelSolvers.emplace_back(make_unique<MetaParallel>(dataA, randomSolverA, LocalSearchA, SEED, -1, timeLimitA));
    metaParallelSolvers.emplace_back(make_unique<MetaParallel>(dataB, randomSolverB, LocalSearchB, SEED, -1, timeLimitB));

    vector<Statistic> scoreStatisticsMetaParallel;
    scoreStatisticsMetaParallel.reserve(metaParallelSolvers.size());
    vector<Statistic> timeStatisticsMetaParallel;
    timeStatisticsMetaParallel.reserve(metaParallelSolvers.size());
    vector<Statistic> iterationStatisticsMetaParallel;
    iterationStatisticsMetaParallel.reserve(metaParallelSolvers.size());
    for (auto &solver : metaParallelSolvers)
    {
        scoreStatisticsMetaParallel.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        timeStatisticsMetaParallel.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
        iterationStatisticsMetaParallel.emplace_back(
            solver->data->getName(),
            solver->getAlgorithmName());
    }
    for (int run = 0; run < numRuns; run++)
    {
        for (size_t i = 0; i < metaParallelSolvers.size(); i++)
        {
            auto &solver = metaParallelSolvers[i];

            println("{} - {} - {}", run, solver->data->getName(), solver->getAlgorithmName());

            startTime = chrono::steady_clock::now();
            solver->solve();
            endTime = chrono::steady_clock::now();
            
            int bestSolutionScore = solver->bestSolutionScore;

            if (bestSolutionScore > scoreStatisticsMetaParallel[i].max)
            {
                solver->saveToFile(format("{}_{}", solver->data->getName(), solver->getAlgorithmName()));
            }

            scoreStatisticsMetaParallel[i].update(bestSolutionScore);
            timeStatisticsMetaParallel[i].update(chrono::duration<double, std::milli>(endTime - startTime).count());
            iterationStatisticsMetaParallel[i].update(solver->currentIterations);
        }
    }
    for (auto &stat : scoreStatisticsMetaParallel)
        stat.average /= numRuns;
    for (auto &stat : timeStatisticsMetaParallel)
        stat.average /= numRuns;
    for (auto &stat : iterationStatisticsMetaParallel)
        stat.average /= numRuns;

    auto allScoreStatistics = {
        // MSLSscoreStatistics,
        scoreStatistics,
        scoreStatisticsEvolution,
        scoreStatisticsALNS,
        scoreStatisticsMetaParallel
    };

    println("\nScore statistics:");
    for(const auto &stat : allScoreStatistics | views::join)
        stat.print();

    auto allTimeStatistics = {
        // MSLStimeStatistics,
        timeStatistics,
        timeStatisticsEvolution,
        timeStatisticsALNS,
        timeStatisticsMetaParallel
    };

    println("\nTime statistics:");
    for(const auto &stat : allTimeStatistics | views::join)
        stat.print();

    auto allIterationStatistics = {
        // MSLSiterationStatistics,
        iterationStatistics,
        iterationStatisticsEvolution,
        iterationStatisticsALNS,
        iterationStatisticsMetaParallel
    };

    println("\nIteration statistics:");
    for(const auto &stat : allIterationStatistics | views::join)
        stat.print();

    return 0;
}