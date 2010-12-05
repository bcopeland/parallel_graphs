#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/delta_stepping_shortest_paths.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/metis.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "metis_dist_mod.hpp"

using namespace boost;
using boost::graph::distributed::mpi_process_group;
using boost::posix_time::ptime;
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

    ptime now(boost::posix_time::microsec_clock::universal_time());
    if (process_id(pg) == 0)
        std::cout << "start: " << now << std::endl;

    std::ifstream in_partitions(argv[2]);

#if USE_SLOW_METIS
    metis_distribution dist(in_partitions, process_id(pg));
#else
    metis_distribution_mod dist(in_partitions, pg);
#endif

    graph_t g(reader.begin(), reader.end(),
            reader.num_vertices(), pg, dist);

    my_id = process_id(g.process_group());

    ptime load_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
        std::cout << "graph loaded in " << (load_time - now) << std::endl;

    ptime start_time(boost::posix_time::microsec_clock::universal_time());
    int start = atoi(argv[3]);

    vertex_t vstart = vertex(start, g);
    delta_stepping_shortest_paths(g, vstart,
              dummy_property_map(),
              get(&vertex_properties::distance, g),
              get(&weighted_edge::weight, g));

    synchronize(g.process_group());

    ptime end_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
        std::cout << "ran in " << (end_time-start_time) << " s\n";

#if 0
    for (i=0 ; i < reader.num_vertices(); i++)
    {
        vertex_t v = vertex(i, g);
        if (owner(v) == my_id)
        {
            int distance = get(get(&vertex_properties::distance, g), v);
            std::cout << i << ": " << distance << std::endl;
        }
    }
#endif


    return 0;
}
