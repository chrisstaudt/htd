#include "simple_graph_reader.h"

#include <htd/MinDegreeTreeDecomposition.hpp>

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char ** argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <graph.gr>" << std::endl;
        return 1;
    }

    try
    {
        SimpleGraph graph = read_pace_graph_mmap(argv[1]);

        if (graph.node_count < 0)
        {
            std::cerr << "Invalid node count" << std::endl;
            return 1;
        }

        std::vector<std::vector<std::size_t> > edges;
        edges.reserve(graph.edges.size() / 2);

        for (std::size_t i = 0; i + 1 < graph.edges.size(); i += 2)
        {
            std::size_t u = static_cast<std::size_t>(graph.edges[i]);
            std::size_t v = static_cast<std::size_t>(graph.edges[i + 1]);

            if (u == 0 || v == 0)
            {
                continue;
            }

            --u;
            --v;

            if (u >= static_cast<std::size_t>(graph.node_count) || v >= static_cast<std::size_t>(graph.node_count))
            {
                continue;
            }

            std::vector<std::size_t> edge;
            edge.push_back(u);
            edge.push_back(v);
            edges.push_back(edge);
        }

        std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

        htd::MinDegreeDecomposition decomposition =
            htd::computeMinDegreeTreeDecomposition(static_cast<std::size_t>(graph.node_count), edges);

        std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();

        long runtimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "tree_width=" << decomposition.width << std::endl;
        std::cout << "runtime_ms=" << runtimeMs << std::endl;

        return 0;
    }
    catch (const std::exception & exception)
    {
        std::cerr << "Error: " << exception.what() << std::endl;
        return 1;
    }
}
