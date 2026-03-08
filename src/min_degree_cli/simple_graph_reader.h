#ifndef SIMPLE_GRAPH_READER_H
#define SIMPLE_GRAPH_READER_H

#include <cstdint>
#include <string>
#include <vector>

struct SimpleGraph
{
    int node_count;
    int edge_count;
    std::vector<uint32_t> edges; // flat array: [tail1, head1, tail2, head2, ...]
};

SimpleGraph read_pace_graph_mmap(const std::string & file_name);

#endif
