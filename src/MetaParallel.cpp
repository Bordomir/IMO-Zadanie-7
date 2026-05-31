#include "../include/MetaParallel.hpp"

#include <thread>
#include <filesystem>
#include <fstream>
#include <format>

#include "../include/ILS.hpp"
#include "../include/ILSv2.hpp"
#include "../include/LNS.hpp"


namespace
{
constexpr int NUM_ILS = 2;
constexpr int NUM_ILSv2 = 2;
constexpr int NUM_LNS = 1;
}

MetaParallel::MetaParallel(DataLoader& data,
                           RandomSolver& randomSolver,
                           MemorySteepLocalSearch& localSearch,
                           unsigned int seed,
                           int maxIterations,
                           double timeLimit)
{
    this->data = &data;
    this->bestSolutionScore = 0;
    this->currentIterations = 0;

    for (int i = 0; i < NUM_ILS; ++i)
    {
        auto randomSolverCopy = std::make_unique<RandomSolver>(randomSolver);
        auto localSearchCopy = std::make_unique<MemorySteepLocalSearch>(localSearch);
        algorithms_.push_back(std::make_unique<ILS>(data, *randomSolverCopy, *localSearchCopy, seed + i, maxIterations, timeLimit, true, true));
        randomSolvers_.push_back(std::move(randomSolverCopy));
        localSearches_.push_back(std::move(localSearchCopy));
    }

    for (int i = 0; i < NUM_ILSv2; ++i)
    {
        auto randomSolverCopy = std::make_unique<RandomSolver>(randomSolver);
        auto localSearchCopy = std::make_unique<MemorySteepLocalSearch>(localSearch);
        algorithms_.push_back(std::make_unique<ILSv2>(data, *randomSolverCopy, *localSearchCopy, seed + NUM_ILS + i, maxIterations, timeLimit, true, true));
        randomSolvers_.push_back(std::move(randomSolverCopy));
        localSearches_.push_back(std::move(localSearchCopy));
    }

    for (size_t i = 0; i < NUM_LNS; ++i)
    {
        auto randomSolverCopy = std::make_unique<RandomSolver>(randomSolver);
        auto localSearchCopy = std::make_unique<MemorySteepLocalSearch>(localSearch);
        algorithms_.push_back(std::make_unique<LNS>(data, *randomSolverCopy, *localSearchCopy, seed + NUM_ILS + NUM_ILSv2 + i, 30, maxIterations, timeLimit, true, true));
        randomSolvers_.push_back(std::move(randomSolverCopy));
        localSearches_.push_back(std::move(localSearchCopy));
    }
}

void MetaParallel::solve()
{
    bestSolutionScore = numeric_limits<int>::min();
    currentIterations = 0;
    bestSolution_.clear();

    {
        std::vector<std::jthread> workers;

        for (const auto& algorithm : algorithms_)
        {
            workers.emplace_back(&AdvancedLocalSearch::solve, algorithm.get());
        }
    }

    for (const auto& algorithm : algorithms_)
    {
        if (algorithm->bestSolutionScore > bestSolutionScore)
        {
            bestSolutionScore = algorithm->bestSolutionScore;
            bestSolution_ = algorithm->bestSolution;
        }
        currentIterations += algorithm->currentIterations;
    }
}

std::string MetaParallel::getAlgorithmName() const
{
    return format("Parallel_{}ILS_{}ILSv2_{}LNS", NUM_ILS, NUM_ILSv2, NUM_LNS);
}

void MetaParallel::saveToFile(const std::string& filename) const
{
    std::filesystem::path dir = "../data/solutions";
    create_directories(dir);
    string fullPath = (dir / format("{}_{}.txt", filename, getAlgorithmName())).string();
    ofstream file(fullPath);
    println(file, "{}", bestSolutionScore);
    for (size_t i = 0; i < bestSolution_.size(); i++)
    {
        println(file, "{}", bestSolution_[i]);
    }
    file.close();
}
