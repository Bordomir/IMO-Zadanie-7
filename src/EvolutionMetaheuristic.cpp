#include "../include/EvolutionMetaheuristic.hpp"
#include "../include/Common.hpp"

#include <limits>
#include <chrono>
#include <vector>
#include <print>
#include <filesystem>
#include <string>
#include <format>
#include <fstream>
#include <random>
#include <utility>

using namespace std;
using namespace filesystem;

constexpr int K = 2;
struct Insertion { size_t pos; int cost; };
struct Subpath { size_t start; int score; };

EvolutionMetaheuristic::EvolutionMetaheuristic(DataLoader &data, RandomSolver &randomSolver, MemorySteepLocalSearch &localSearch, int eliteSize, int maxIterations, double timeLimit, bool usesLocalSearch, unsigned int seed) : data(&data), randomSolver(&randomSolver), LocalSearch(&localSearch), rng(seed)
{
    if (maxIterations > 0)
    {
        this->usesTimeLimit = false;
        this->maxIterations = maxIterations;
    }
    else if (timeLimit > 0)
    {
        this->usesTimeLimit = true;
        this->timeLimit = timeLimit;
    }
    else
    {
        this->usesTimeLimit = false;
        this->maxIterations = 200;
    }
    this->eliteSize = eliteSize;
    this->usesLocalSearch = usesLocalSearch;
    setData(data);
}

void EvolutionMetaheuristic::setData(DataLoader &data)
{
    this->randomSolver->data = &data;
    this->LocalSearch->data = &data;
}

void EvolutionMetaheuristic::solve()
{
    startTime = chrono::steady_clock::now();
    bestSolutions = vector<vector<int>>();
    bestSolutions.clear();
    bestSolutionScores = vector<int>();
    bestSolutionScores.clear();
    for(int i = 0; i < eliteSize; i++)
    {
        vector<int> solution;
        int solutionScore;
        bool isDifferent = false;
        while(!isDifferent)
        {
            solution = getRandomizedSolution();
            solution = improveSolution(solution);
            solutionScore = LocalSearch->solutionScore;
            
            isDifferent = isSolutionDifferent(solutionScore);
        }

        bestSolutions.push_back(solution);
        bestSolutionScores.push_back(solutionScore);
    }
    currentIterations = 0;
    while(canContinue())
    {
        int firstParentIndex = randomInt(0, eliteSize - 1);
        int secondParentIndex = randomInt(0, eliteSize - 1);
        while(firstParentIndex == secondParentIndex)
        {
            secondParentIndex = randomInt(0, eliteSize - 1);
        }
        vector<int> firstParent = bestSolutions[firstParentIndex];
        vector<int> secondParent = bestSolutions[secondParentIndex];

        vector<int> solution = recombineParents(firstParent, secondParent);
        repair(solution);
        int solutionScore;
        if(usesLocalSearch)
        {
            solution = improveSolution(solution);
            solutionScore = LocalSearch->solutionScore;
        }
        else
        {
            solutionScore = calculateScore(solution);
        }

        int worstSolutionIndex = 0;
        for (int i = 1; i < eliteSize; i++)
        {
            if(bestSolutionScores[i] < bestSolutionScores[worstSolutionIndex]){
                worstSolutionIndex = i;
            }
        }

        if(solutionScore > bestSolutionScores[worstSolutionIndex] && isSolutionDifferent(solutionScore))
        {
            bestSolutions[worstSolutionIndex] = solution;
            bestSolutionScores[worstSolutionIndex] = solutionScore;
        }
        currentIterations++;
    }
}

vector<int> EvolutionMetaheuristic::getRandomizedSolution() const
{
    randomSolver->solve();
    return randomSolver->solution;
}

vector<int> EvolutionMetaheuristic::improveSolution(vector<int> &solution) const
{
    LocalSearch->solution = solution;
    LocalSearch->improve();
    return LocalSearch->solution;
}

bool EvolutionMetaheuristic::canContinue() const
{
    if (usesTimeLimit)
    {
        auto currentTime = chrono::steady_clock::now();
        auto elapsedTime = chrono::duration<double, std::milli>(currentTime-startTime).count();
        return elapsedTime < timeLimit;
    }
    else
    {
        return currentIterations < maxIterations;
    }
}

bool EvolutionMetaheuristic::isSolutionDifferent(int solutionScore) const
{
    for(size_t i = 0; i < bestSolutionScores.size(); i++)
    {
        if(solutionScore == bestSolutionScores[i])
        {
            return false;
        }
    } 
    return true;
}
bool EvolutionMetaheuristic::isSolutionDifferent(vector<int> &solution) const
{
    int solutionScore = calculateScore(solution);
    return isSolutionDifferent(solutionScore);
}
void EvolutionMetaheuristic::repair(vector<int> &solution)
{
    
    if (solution.empty())
    {
        solution.push_back(randomInt(0, data->numNodes - 1));
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
vector<pair<int, int>> EvolutionMetaheuristic::makeHelperArray(vector<int> &solution) const
{
    vector<pair<int, int>> helperArray(data->numNodes, make_pair<int, int>(-1, -1));
    int n = solution.size();
    int node = solution[0];
    int prev = solution[n-1];
    int next = solution[1];
    helperArray[node].first = prev;
    helperArray[node].second = next;
    node = solution[n-1];
    prev = solution[n-2];
    next = solution[0];
    helperArray[node].first = prev;
    helperArray[node].second = next;
    for (size_t i = 1; i < solution.size()-1; ++i)
    {
        node = solution[i];
        prev = solution[i-1];
        next = solution[i+1];
        helperArray[node].first = prev;
        helperArray[node].second = next;
    }
    return helperArray;
}

vector<int> EvolutionMetaheuristic::makeInSolutionArray(vector<int> &solution) const
{
    vector<int> inSolution(data->numNodes, -1);
    for (size_t i = 0; i < solution.size(); ++i)
    {
        inSolution[solution[i]] = i;
    }
    return inSolution;
}
int EvolutionMetaheuristic::calculateScore(const vector<int> &solution) const
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
int EvolutionMetaheuristic::getBestSolutionIndex() const
{
    int bestSolutionIndex = 0;    
    int bestSolutionScore = bestSolutionScores[0];
    for (size_t i = 1; i < bestSolutions.size(); i++)
    {
        if(bestSolutionScores[i] > bestSolutionScore){
            bestSolutionIndex = i;
            bestSolutionScore = bestSolutionScores[i];
        }
    }
    return bestSolutionIndex;
}

void EvolutionMetaheuristic::print() const
{
    int bestSolutionIndex = getBestSolutionIndex();
    vector<int> bestSolution = bestSolutions[bestSolutionIndex];
    int bestSolutionScore = bestSolutionScores[bestSolutionIndex];
    println("Trasa:\n{}", bestSolution);
    println("Funkcja celu: {}", bestSolutionScore);
}

void EvolutionMetaheuristic::saveToFile(const string &filename) const
{
    int bestSolutionIndex = getBestSolutionIndex();
    vector<int> bestSolution = bestSolutions[bestSolutionIndex];
    int bestSolutionScore = bestSolutionScores[bestSolutionIndex];
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
int EvolutionMetaheuristic::randomInt(int min, int max)
{
    uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}