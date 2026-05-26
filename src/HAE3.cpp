#include "../include/HAE3.hpp"

#include <format>

std::string HAE3::getAlgorithmName() const
{
    return std::format("HAE3_{}", usesLocalSearch ? "withIntermediateLS" : "noIntermediateLS");
}

std::vector<int> HAE3::recombineParents(std::vector<int>& firstParent, std::vector<int>& secondParent)
{
    std::vector<int> inSecondParent = makeInSolutionArray(secondParent);
    std::vector<int> recombinedSolution;

    for (int node : firstParent)
    {
        if (inSecondParent[node] != -1)
        {
            recombinedSolution.push_back(node);
        }
    }

    return recombinedSolution;
}
