#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/compressed_sparse_row_graph.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/graph/distributed/concepts.hpp>
#include <boost/graph/distributed/betweenness_centrality.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/metis.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "metis_dist_mod.hpp"

using namespace boost;
using boost::graph::distributed::mpi_process_group;
using boost::posix_time::ptime;
using namespace boost::graph;

#if 0
typedef adjacency_list<vecS, distributedS<mpi_process_group, vecS>,
                       directedS,
                       no_property,
                       property<edge_weight_t, int> > graph_t;
#else

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

typedef compressed_sparse_row_graph<directedS,
                       no_property, weighted_edge,
                       no_property, distributedS<mpi_process_group> >
                       graph_t;

#endif

typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef property_map<graph_t, vertex_index_t>::const_type index_map_t;
typedef iterator_property_map<std::vector<int>::iterator, index_map_t>
    centrality_map_t;

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

    std::vector<int> centralityS(num_vertices(g), 0);
    centrality_map_t centrality(centralityS.begin(), get(vertex_index, g));

    my_id = process_id(g.process_group());

    ptime load_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
        std::cout << "graph loaded in " << (load_time - now) << std::endl;

    ptime start_time(boost::posix_time::microsec_clock::universal_time());
    int start = atoi(argv[3]);

    brandes_betweenness_centrality(g, centrality);

    synchronize(g.process_group());

    ptime end_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
        std::cout << "ran in " << (end_time-start_time) << " s\n";

    property_map<graph_t, vertex_owner_t>::const_type owner =
        get(vertex_owner, g);

    for (i=0 ; i < reader.num_vertices(); i++)
    {
        vertex_t v = vertex(i, g);
        if (get(owner, v) == my_id)
        {
            int distance = get(centrality, v);
            std::cout << i << ": " << distance << std::endl;
        }
    }

    return 0;
}
