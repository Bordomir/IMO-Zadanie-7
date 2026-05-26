#include "../include/ALNS.hpp"
#include "../include/Common.hpp"

#include <algorithm>
#include <numeric>
#include <print>
#include <filesystem>
#include <string>
#include <format>
#include <fstream>
#include <chrono>
#include <mutex>
#include <random>


using namespace std;
using namespace filesystem;

namespace
{
constexpr int K = 2;
struct Insertion { size_t pos; int cost; };
struct Subpath { size_t start; int score; };
}

void ALNS::solve()
{
}

void ALNS::destroySmart(vector<int> &solution, mt19937& rng)
{
}

void ALNS::destroyHeuristic(vector<int> &solution, mt19937& rng)
{
    if (solution.empty())
    {
        return;
    }

    int nNodesToRemove = solution.size() * destructionPercentage / 100;

    if (nNodesToRemove == 0)
    {
        return;
    }

    if (nNodesToRemove == static_cast<int>(solution.size()))
    {
        solution.clear();
        return;
    }

    if (nNodesToRemove == 1)
    {
        solution.erase(solution.begin() + randomInt(0, solution.size() - 1, rng));
        return;
    }

    size_t subpathLength = randomInt(2, nNodesToRemove, rng);

    std::vector<Subpath> subpaths;
    subpaths.reserve(solution.size());

    int currentScore = data->nodeProfits[solution[0]];

    for(size_t i = 1; i < subpathLength; ++i)
    {
        currentScore += data->nodeProfits[solution[i]]
                        - data->distanceMatrix[solution[i - 1]][solution[i]];
    }

    subpaths.push_back({0, currentScore});

    for (size_t left = 1, right = subpathLength; left < solution.size(); ++left, ++right)
    {
        size_t prevRight = (right - 1) % solution.size();
        size_t currRight = right % solution.size();
        size_t prevLeft = (left - 1) % solution.size();
        size_t currLeft = left % solution.size();

        currentScore += data->nodeProfits[solution[currRight]]
                        - data->distanceMatrix[solution[prevRight]][solution[currRight]]
                        - data->nodeProfits[solution[prevLeft]]
                        + data->distanceMatrix[solution[prevLeft]][solution[currLeft]];

        subpaths.push_back({left, currentScore});
    }

    int maxScore = std::max_element(
        subpaths.begin(),
        subpaths.end(),
        [](const Subpath& a, const Subpath& b)
        {
            return a.score < b.score;
        })->score;

    std::vector<double> weights;
    weights.reserve(subpaths.size());

    for (const Subpath& subpath : subpaths)
    {
        weights.push_back(maxScore - subpath.score);
    }

    std::vector<bool> toRemove(solution.size(), false);

    while (nNodesToRemove > 0)
    {
        std::discrete_distribution<size_t> dist(weights.begin(), weights.end());
        const Subpath& subpath = subpaths[dist(rng)];

        for (size_t j = 0; j < subpathLength && nNodesToRemove > 0; ++j)
        {
            size_t k = (subpath.start + j) % solution.size();

            if (not toRemove[k])
            {
                toRemove[k] = true;
                --nNodesToRemove;
            }
        }

        weights[subpath.start] = 0.0;
    }

    std::vector<int> reducedSolution;
    reducedSolution.reserve(solution.size());

    for (size_t i = 0; i < solution.size(); ++i)
    {
        if (!toRemove[i])
        {
            reducedSolution.push_back(solution[i]);
        }
    }

    solution = std::move(reducedSolution);
}

void ALNS::repair(vector<int> &solution, mt19937& rng)
{
    if (solution.empty())
    {
        solution.push_back(randomInt(0, data->numNodes - 1, rng));
    }

    std::vector<bool> visited(data->numNodes, false);

    for (int node : solution)
    {
        visited[node] = true;
    }

    for (int i = solution.size(); i < data->numNodes; ++i)
    {
        int bestNode = -1;
        size_t bestPos = 0;
        int bestCost = std::numeric_limits<int>::max();
        int bestRegret = std::numeric_limits<int>::min();

        for (int currNode = 0; currNode < data->numNodes; ++currNode)
        {
            if (visited[currNode]) continue;

            std::vector<Insertion> insertions;
            insertions.reserve(solution.size());

            for (size_t prevIndex = 0; prevIndex < solution.size(); ++prevIndex)
            {
                size_t nextIndex = (prevIndex + 1) % solution.size();

                int prevNode = solution[prevIndex];
                int nextNode = solution[nextIndex];

                int cost = data->distanceMatrix[prevNode][currNode]
                           + data->distanceMatrix[currNode][nextNode]
                           - data->distanceMatrix[prevNode][nextNode]
                           - data->nodeProfits[currNode];

                insertions.emplace_back(prevIndex + 1, cost);
            }

            int kElements = std::min(K, static_cast<int>(insertions.size()));

            std::partial_sort(
                insertions.begin(),
                insertions.begin() + kElements,
                insertions.end(),
                [](const Insertion& a, const Insertion& b)
                {
                    return a.cost < b.cost;
                });

            int regret = 0;

            for (int j = 1; j < kElements; ++j)
            {
                regret += insertions[j].cost - insertions[0].cost;
            }

            if (regret > bestRegret || (regret == bestRegret && insertions[0].cost < bestCost))
            {
                bestNode = currNode;
                bestPos = insertions[0].pos;
                bestCost = insertions[0].cost;
                bestRegret = regret;
            }
        }

        solution.insert(solution.begin() + bestPos, bestNode);
        visited[bestNode] = true;
    }

    common::improveByRemovingNodes(solution, data);
}

int ALNS::randomInt(int min, int max, mt19937 &rng)
{
    uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}
string ALNS::getAlgorithmName() const
{
    return "ALNS";
}

int ALNS::calculateScore(const vector<int> &solution) const
{
    int score = 0;
    if (solution.size() == 0)
    {
        return score;
    }
    // + zysk z odwiedzenia pierwszego wierzchołka
    score = data->nodeProfits[solution[0]];
    for (size_t currentNode = 1; currentNode < solution.size(); currentNode++)
    {
        // - odległość currentNode - 1 -> currentNode
        score -= data->distanceMatrix[solution[currentNode - 1]][solution[currentNode]];
        // + zysk z odwiedzenia wierzchołka currentNode
        score += data->nodeProfits[solution[currentNode]];
    }
    if (solution.size() > 1)
    {
        // - odległość lastNode -> currentNode
        score -= data->distanceMatrix[solution[solution.size() - 1]][solution[0]];
    }
    return score;
}

void ALNS::print() const
{
    println("Trasa:\n{}", bestSolution);
    println("Funkcja celu: {}", bestSolutionScore);
}

void ALNS::saveToFile(const string &filename) const
{
    path dir = "../data/solutions";
    create_directories(dir);
    string fullPath = (dir / format("{}_{}.txt", filename, getAlgorithmName())).string();
    ofstream file(fullPath);
    println(file, "{}", bestSolutionScore);
    for (size_t i = 0; i < bestSolution.size(); i++)
    {
        println(file, "{}", bestSolution[i]);
    }
    file.close();
}
