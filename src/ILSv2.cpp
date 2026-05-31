#include "../include/ILSv2.hpp"

#include <string>
#include <vector>
#include <random>
#include <algorithm>


std::string ILSv2::getAlgorithmName() const
{
    return "ILSv2";
}

std::vector<int> ILSv2::createNewSolution()
{
    std::vector<int> newSolution(bestSolution);

    if (newSolution.size() < 4) return newSolution;

    std::vector<int> inSolution(data->numNodes, -1);
    for (size_t i = 0; i < newSolution.size(); i++)
    {
        inSolution[newSolution[i]] = i;
    }

    int outMoves = randomInt(2, 5);
    for (int i = 0; i < outMoves; i++)
    {
        if (newSolution.empty()) break;
        int outIdx = randomInt(0, newSolution.size() - 1);
        int outNode = newSolution[outIdx];

        int inNode;
        do
        {
            inNode = randomInt(0, data->numNodes - 1);
        } while (inSolution[inNode] != -1);

        inSolution[outNode] = -1;
        newSolution[outIdx] = inNode;
        inSolution[inNode] = outIdx;
    }

    int relocateMoves = randomInt(1, 3);
    for (int i = 0; i < relocateMoves; i++)
    {
        if (newSolution.size() < 4) break;
        int length = randomInt(2, min(5, (int)newSolution.size() - 2));
        int startPos = randomInt(0, newSolution.size() - length);

        std::vector<int> subpath(newSolution.begin() + startPos, newSolution.begin() + startPos + length);
        newSolution.erase(newSolution.begin() + startPos, newSolution.begin() + startPos + length);

        int insertPos = randomInt(0, newSolution.size());
        newSolution.insert(newSolution.begin() + insertPos, subpath.begin(), subpath.end());
    }

    return newSolution;
}

int ILSv2::randomInt(int min, int max)
{
    std::uniform_int_distribution<int> dist(min, max);
    return dist(rng);
}
