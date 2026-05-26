#include "../include/HAE2.hpp"

#include <string>
#include <vector>
#include <utility>
#include <numeric>
#include <random>
#include <format>

using namespace std;

string HAE2::getAlgorithmName() const
{
    return format("HAE2_{}", usesLocalSearch ? "withIntermediateLS" : "noIntermediateLS");
}

vector<int> HAE2::recombineParents(vector<int> &firstParent, vector<int> &secondParent)
{
    vector<int> inSolution = makeInSolutionArray(secondParent);
    vector<pair<int, int>> edges = makeHelperArray(secondParent);

    int n = firstParent.size();

    vector<int> breakPoints;
    vector<int> sharedNodesSolution;
    for(int i = 0; i < n; i++)
    {
        int current = firstParent[i];
        if(inSolution[current] > -1)
        {
            sharedNodesSolution.push_back(current);
            int sharedNodes = sharedNodesSolution.size();
            if(sharedNodes > 1)
            {
                int last = sharedNodesSolution[sharedNodes - 2];
                if(!(edges[last].first == current || edges[last].second == current))
                {
                    breakPoints.push_back(sharedNodes - 1);
                }
            }
        }
    }
    if(sharedNodesSolution.empty())
    {
        return vector<int>();
    }

    int sharedNodes = sharedNodesSolution.size();
    int firstNode = sharedNodesSolution[0];
    int lastNode = sharedNodesSolution[sharedNodes-1];
    if(!(edges[firstNode].first == lastNode || edges[firstNode].second == lastNode))
    {
        breakPoints.push_back(sharedNodes);
    }

    if(breakPoints.size() <= 1)
    {
        return sharedNodesSolution;
    }

    int breakPointsSize = breakPoints.size();
    vector<vector<int>> subpaths;
    vector<int> subpath;
    int start = breakPoints[0];
    int end;
    for(int i = 1; i < breakPointsSize; i++)
    {
        end = breakPoints[i];
        for(int j = start; j < end; j++)
        {
            subpath.push_back(sharedNodesSolution[j]);
        }
        
        if(subpath.size() > 1)
        {
            subpaths.push_back(subpath);
        }

        start = end;
        subpath.clear();
    }
    for(int i = start; i < sharedNodes; i++)
    {
        subpath.push_back(sharedNodesSolution[i]);
    }
    end = breakPoints[0];
    for(int i = 0; i < end; i++)
    {
        subpath.push_back(sharedNodesSolution[i]);
    }
    if(subpath.size() > 1)
    {
        subpaths.push_back(subpath);
    }
    if(subpaths.empty())
    {
        return vector<int>();
    }

    vector<int> subpathJoiningIndices(subpaths.size());
    iota(subpathJoiningIndices.begin(), subpathJoiningIndices.end(), 0);

    shuffle(subpathJoiningIndices.begin(), subpathJoiningIndices.end(), rng);

    bernoulli_distribution coinFlip(0.5);
    vector<bool> isSubpathsReversed(subpaths.size());
    for(size_t i = 0; i < isSubpathsReversed.size(); i++)
    {
        isSubpathsReversed[i] = coinFlip(rng);
    }

    vector<int> recombinedSolution;
    recombinedSolution.reserve(sharedNodesSolution.size());
    for(size_t i = 0; i < subpathJoiningIndices.size(); i++)
    {
        int subpathIndex = subpathJoiningIndices[i];
        vector<int> subpath = subpaths[subpathIndex];
        int subpathSize = subpath.size();
        if(isSubpathsReversed[subpathIndex])
        {
            for(int j = subpathSize - 1; j > -1; j--)
            {
                recombinedSolution.push_back(subpath[j]);
            }
        }
        else
        {
            for(int j = 0; j < subpathSize; j++)
            {
                recombinedSolution.push_back(subpath[j]);
            }
        }
    }

    return recombinedSolution;
}
