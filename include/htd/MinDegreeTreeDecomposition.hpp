/*
 * File:   MinDegreeTreeDecomposition.hpp
 *
 * Standalone minimum-degree decomposition implementation without htd dependencies.
 */

#ifndef HTD_HTD_MINDEGREETREEDECOMPOSITION_HPP
#define HTD_HTD_MINDEGREETREEDECOMPOSITION_HPP

#include <algorithm>
#include <cstddef>
#include <queue>
#include <unordered_set>
#include <utility>
#include <vector>

namespace htd
{
    struct MinDegreeDecomposition
    {
        std::size_t width;
        std::vector<std::vector<std::size_t> > bags;
        std::vector<std::pair<std::size_t, std::size_t> > treeEdges;
    };

    inline MinDegreeDecomposition computeMinDegreeTreeDecomposition(
        std::size_t vertexCount,
        const std::vector<std::vector<std::size_t> > & hyperedges)
    {
        std::vector<std::unordered_set<std::size_t> > adjacency(vertexCount);

        for (const std::vector<std::size_t> & edge : hyperedges)
        {
            for (std::size_t i = 0; i < edge.size(); ++i)
            {
                std::size_t u = edge[i];

                if (u >= vertexCount)
                {
                    continue;
                }

                for (std::size_t j = i + 1; j < edge.size(); ++j)
                {
                    std::size_t v = edge[j];

                    if (v >= vertexCount || u == v)
                    {
                        continue;
                    }

                    adjacency[u].insert(v);
                    adjacency[v].insert(u);
                }
            }
        }

        std::vector<unsigned char> alive(vertexCount, 1);
        std::vector<std::size_t> eliminationOrder;
        eliminationOrder.reserve(vertexCount);

        std::vector<std::vector<std::size_t> > bags;
        bags.reserve(vertexCount);

        typedef std::pair<std::size_t, std::size_t> QueueEntry;
        std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry> > queue;

        for (std::size_t v = 0; v < vertexCount; ++v)
        {
            queue.push(std::make_pair(adjacency[v].size(), v));
        }

        std::size_t maxBagSize = 0;

        for (std::size_t eliminated = 0; eliminated < vertexCount; ++eliminated)
        {
            std::size_t vertex = vertexCount;

            while (!queue.empty())
            {
                std::size_t candidate = queue.top().second;
                std::size_t degree = queue.top().first;
                queue.pop();

                if (candidate < vertexCount && alive[candidate] && adjacency[candidate].size() == degree)
                {
                    vertex = candidate;
                    break;
                }
            }

            if (vertex >= vertexCount)
            {
                break;
            }

            std::vector<std::size_t> neighbors(adjacency[vertex].begin(), adjacency[vertex].end());

            std::vector<std::size_t> bag;
            bag.reserve(neighbors.size() + 1);
            bag.push_back(vertex);
            bag.insert(bag.end(), neighbors.begin(), neighbors.end());
            std::sort(bag.begin(), bag.end());
            bags.push_back(bag);

            if (bag.size() > maxBagSize)
            {
                maxBagSize = bag.size();
            }

            for (std::size_t i = 0; i < neighbors.size(); ++i)
            {
                std::size_t u = neighbors[i];

                if (!alive[u])
                {
                    continue;
                }

                for (std::size_t j = i + 1; j < neighbors.size(); ++j)
                {
                    std::size_t w = neighbors[j];

                    if (!alive[w] || u == w)
                    {
                        continue;
                    }

                    adjacency[u].insert(w);
                    adjacency[w].insert(u);
                }
            }

            for (std::size_t u : neighbors)
            {
                adjacency[u].erase(vertex);
                queue.push(std::make_pair(adjacency[u].size(), u));
            }

            adjacency[vertex].clear();
            alive[vertex] = 0;
            eliminationOrder.push_back(vertex);
        }

        std::vector<std::size_t> eliminationPosition(vertexCount, (std::size_t)-1);

        for (std::size_t i = 0; i < eliminationOrder.size(); ++i)
        {
            eliminationPosition[eliminationOrder[i]] = i;
        }

        std::vector<std::size_t> parentPosition(eliminationOrder.size(), (std::size_t)-1);

        for (std::size_t i = 0; i < eliminationOrder.size(); ++i)
        {
            std::size_t vertex = eliminationOrder[i];
            std::size_t best = (std::size_t)-1;

            for (std::size_t candidate : bags[i])
            {
                if (candidate == vertex || candidate >= vertexCount)
                {
                    continue;
                }

                std::size_t position = eliminationPosition[candidate];

                if (position > i && (best == (std::size_t)-1 || position < best))
                {
                    best = position;
                }
            }

            parentPosition[i] = best;
        }

        std::vector<std::pair<std::size_t, std::size_t> > treeEdges;
        treeEdges.reserve(eliminationOrder.size() > 0 ? eliminationOrder.size() - 1 : 0);

        std::size_t root = (std::size_t)-1;

        for (std::size_t i = 0; i < parentPosition.size(); ++i)
        {
            if (parentPosition[i] != (std::size_t)-1)
            {
                treeEdges.push_back(std::make_pair(i, parentPosition[i]));
            }
            else if (root == (std::size_t)-1)
            {
                root = i;
            }
            else
            {
                treeEdges.push_back(std::make_pair(i, root));
            }
        }

        MinDegreeDecomposition result;
        result.width = maxBagSize == 0 ? 0 : maxBagSize - 1;
        result.bags.swap(bags);
        result.treeEdges.swap(treeEdges);

        return result;
    }
}

#endif /* HTD_HTD_MINDEGREETREEDECOMPOSITION_HPP */
