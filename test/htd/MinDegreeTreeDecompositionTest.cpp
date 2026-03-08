#include <gtest/gtest.h>

#include <htd/MinDegreeTreeDecomposition.hpp>

#include <algorithm>
#include <queue>

namespace
{
    bool hasVertexInBag(const std::vector<std::size_t> & bag, std::size_t vertex)
    {
        return std::binary_search(bag.begin(), bag.end(), vertex);
    }

    bool checkRunningIntersection(const htd::MinDegreeDecomposition & decomposition, std::size_t vertexCount)
    {
        std::vector<std::vector<std::size_t> > bagAdj(decomposition.bags.size());

        for (const std::pair<std::size_t, std::size_t> & edge : decomposition.treeEdges)
        {
            bagAdj[edge.first].push_back(edge.second);
            bagAdj[edge.second].push_back(edge.first);
        }

        for (std::size_t vertex = 0; vertex < vertexCount; ++vertex)
        {
            std::vector<std::size_t> containing;

            for (std::size_t i = 0; i < decomposition.bags.size(); ++i)
            {
                if (hasVertexInBag(decomposition.bags[i], vertex))
                {
                    containing.push_back(i);
                }
            }

            if (containing.empty())
            {
                continue;
            }

            std::vector<unsigned char> visited(decomposition.bags.size(), 0);
            std::queue<std::size_t> queue;
            queue.push(containing.front());
            visited[containing.front()] = 1;

            while (!queue.empty())
            {
                std::size_t current = queue.front();
                queue.pop();

                for (std::size_t next : bagAdj[current])
                {
                    if (!visited[next] && hasVertexInBag(decomposition.bags[next], vertex))
                    {
                        visited[next] = 1;
                        queue.push(next);
                    }
                }
            }

            for (std::size_t bagIndex : containing)
            {
                if (!visited[bagIndex])
                {
                    return false;
                }
            }
        }

        return true;
    }
}

TEST(MinDegreeTreeDecompositionTest, ComputesExpectedWidthForPath)
{
    std::vector<std::vector<std::size_t> > edges;
    edges.push_back({0, 1});
    edges.push_back({1, 2});
    edges.push_back({2, 3});
    edges.push_back({3, 4});

    htd::MinDegreeDecomposition result = htd::computeMinDegreeTreeDecomposition(5, edges);

    ASSERT_EQ((std::size_t)1, result.width);
    ASSERT_EQ((std::size_t)5, result.bags.size());
    ASSERT_EQ((std::size_t)4, result.treeEdges.size());
    ASSERT_TRUE(checkRunningIntersection(result, 5));
}

TEST(MinDegreeTreeDecompositionTest, HandlesHyperedgesAndCoversAllEdges)
{
    std::vector<std::vector<std::size_t> > hyperedges;
    hyperedges.push_back({0, 1, 2});
    hyperedges.push_back({2, 3, 4});
    hyperedges.push_back({4, 5});

    htd::MinDegreeDecomposition result = htd::computeMinDegreeTreeDecomposition(6, hyperedges);

    ASSERT_EQ((std::size_t)6, result.bags.size());
    ASSERT_EQ((std::size_t)5, result.treeEdges.size());
    ASSERT_TRUE(checkRunningIntersection(result, 6));

    for (const std::vector<std::size_t> & edge : hyperedges)
    {
        bool covered = false;

        for (const std::vector<std::size_t> & bag : result.bags)
        {
            bool containsAll = true;

            for (std::size_t v : edge)
            {
                if (!hasVertexInBag(bag, v))
                {
                    containsAll = false;
                    break;
                }
            }

            if (containsAll)
            {
                covered = true;
                break;
            }
        }

        ASSERT_TRUE(covered);
    }
}
