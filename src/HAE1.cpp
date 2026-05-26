#include "../include/HAE1.hpp"

#include <format>
#include <numeric>
#include <random>
#include <algorithm>

namespace
{
struct Subpath { int pos; int length; };
}

std::string HAE1::getAlgorithmName() const
{
    return std::format("HAE1_{}", usesLocalSearch ? "withIntermediateLS" : "noIntermediateLS");
}

std::vector<int> HAE1::recombineParents(std::vector<int>& firstParent, std::vector<int>& secondParent)
{
    std::vector<int> inSecondParent = makeInSolutionArray(secondParent);
    std::vector<std::pair<int, int>> prevNextSecond = makeHelperArray(secondParent);
    int firstParentSize = firstParent.size();

    int start = 0;
    for (; start < firstParentSize; ++start)
    {
        int curr = firstParent[start];

        if (inSecondParent[curr] == -1)
        {
            break;
        }

        int prev = firstParent[(start - 1 + firstParentSize) % firstParentSize];

        if (prevNextSecond[curr].first != prev && prevNextSecond[curr].second != prev)
        {
            break;
        }
    }

    std::vector<Subpath> subpaths;
    Subpath currSubpath(-1, -1);

    for (int i = 0; i < firstParentSize; ++i)
    {
        int pos = start + i;
        int curr = firstParent[(pos) % firstParentSize];
        int next = firstParent[(pos + 1) % firstParentSize];

        bool isSubpathOngoing = currSubpath.pos != -1;
        bool isCurrInSecondParent = inSecondParent[curr] != -1;
        bool isNextNeighborInSecondParent = prevNextSecond[curr].first == next || prevNextSecond[curr].second == next;

        if (!isSubpathOngoing && isCurrInSecondParent && !isNextNeighborInSecondParent)
        {
            subpaths.emplace_back(pos, 1);
        }
        else if (!isSubpathOngoing && isCurrInSecondParent && isNextNeighborInSecondParent)
        {
            currSubpath = Subpath(pos, 1);
        }
        else if (isSubpathOngoing && isCurrInSecondParent && !isNextNeighborInSecondParent)
        {
            ++currSubpath.length;
            subpaths.push_back(currSubpath);
            currSubpath = Subpath(-1, -1);
        }
        else if (isSubpathOngoing && isCurrInSecondParent && isNextNeighborInSecondParent)
        {
            ++currSubpath.length;
        }
    }

    std::shuffle(subpaths.begin(), subpaths.end(), rng);
    std::bernoulli_distribution coinFlip(0.5);
    std::vector<int> recombinedSolution;

    for (const Subpath& subpath : subpaths)
    {
        if (coinFlip(rng))
        {
            for (int i = 0; i < subpath.length; ++i)
            {
                int pos = subpath.pos + i;
                recombinedSolution.push_back(firstParent[pos % firstParentSize]);
            }
        }
        else
        {
            for (int i = subpath.length - 1; i >= 0; --i)
            {
                int pos = subpath.pos + i;
                recombinedSolution.push_back(firstParent[pos % firstParentSize]);
            }
        }
    }

    return recombinedSolution;
}
