#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/delta_stepping_shortest_paths.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/metis.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>

using namespace boost;
using boost::graph::distributed::mpi_process_group;
using namespace boost::graph;

struct weighted_edge
{
    weighted_edge(int weight = 1) : weight(weight) {}
    int weight;

    template<typename Archiver>
    void serialize(Archiver &ar, const unsigned int version)
    {
        ar & weight;
    }
};

struct vertex_properties
{
    vertex_properties(int d = 0) : distance(d) { }

    int distance;
    template<typename Archiver>
    void serialize(Archiver &ar, const unsigned int version)
    {
        ar & distance;
    }
};


typedef adjacency_list<vecS, distributedS<mpi_process_group, vecS>,
                       directedS, vertex_properties, weighted_edge> graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef std::pair<int, int> edge_t;
typedef property_map<vertex_t, float> rank_map_t;

int main(int argc, char *argv[])
{
    int i;
    int my_id;

    mpi::environment env(argc, argv);

    std::ifstream in_graph(argv[1]);
    metis_reader reader(in_graph);
    mpi_process_group pg;

    std::ifstream in_partitions(argv[2]);
    metis_distribution dist(in_partitions, process_id(pg));
    graph_t g(reader.begin(), reader.end(),
            reader.num_vertices(), pg, dist);

    int start = atoi(argv[3]);
    my_id = process_id(g.process_group());

    vertex_t vstart = vertex(start, g);
    delta_stepping_shortest_paths(g, vstart,
              dummy_property_map(),
              get(&vertex_properties::distance, g),
              get(&weighted_edge::weight, g));

    for (i=0 ; i < reader.num_vertices(); i++)
    {
        vertex_t v = vertex(i, g);
        if (owner(v) == my_id)
        {
            int distance = get(get(&vertex_properties::distance, g), v);
            std::cout << i << ": " << distance << std::endl;
        }
    }

    return 0;
}