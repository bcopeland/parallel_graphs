#include <boost/graph/use_mpi.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/metis.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include <limits.h>
#include "metis_dist_mod.hpp"


using namespace boost;
using boost::graph::distributed::mpi_process_group;
using boost::posix_time::ptime;
using namespace boost::graph;

typedef adjacency_list<vecS, distributedS<mpi_process_group, vecS>,
                       directedS,
                       property<vertex_distance_t, size_t> > graph_t;

typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef std::pair<int, int> edge_t;
typedef property_map<vertex_t, float> rank_map_t;

int main(int argc, char *argv[])
{
    int i;
    int my_id;
    vertex_t *v;

    mpi::environment env(argc, argv);

    std::ifstream in_graph(argv[1]);
    metis_reader reader(in_graph);
    mpi_process_group pg;

    int start = atoi(argv[3]);

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

    property_map<graph_t, vertex_distance_t>::type distance =
        get(vertex_distance, g);

    BGL_FORALL_VERTICES(v, g, graph_t) 
    {
        put(distance, v, INT_MAX);
    }
    vertex_t vstart = vertex(start, g);
    put(distance, vstart, 0);

    ptime load_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
        std::cout << "graph loaded in " << (load_time - now) << std::endl;

    ptime start_time(boost::posix_time::microsec_clock::universal_time());

    breadth_first_search(g, vstart,
        visitor(make_bfs_visitor(record_distances(distance, on_tree_edge()))));

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
            size_t distance = get(get(vertex_distance, g), v);
            std::cout << i << ": " << distance << std::endl;
        }
    }
#endif


    return 0;
}
