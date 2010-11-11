#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/page_rank.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/random/linear_congruential.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>

using namespace boost;
using boost::graph::distributed::mpi_process_group;

using namespace boost::graph;
typedef adjacency_list<vecS, distributedS<mpi_process_group, vecS>,
                       directedS> graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef std::pair<int, int> edge_t;
typedef property_map<vertex_t, float> rank_map_t;

int main(int argc, char *argv[])
{
    int i;
    int dim = 11;
    double initial_rank[dim];
    int my_id;

    mpi::environment env(argc, argv);

    for (i = 0; i < dim; i++)
        initial_rank[i] = 1.0/dim;

    edge_t edge_array[] =
    {
        edge_t(0,1), edge_t(0,2), edge_t(0,3), edge_t(0,4),
        edge_t(0,5), edge_t(0,6), edge_t(0,7), edge_t(0,8),
        edge_t(0,9), edge_t(0,10), edge_t(0,0),
        edge_t(1,2),
        edge_t(2,1),
        edge_t(3,0), edge_t(3,1),
        edge_t(4,1), edge_t(4,3), edge_t(4,5),
        edge_t(4,1), edge_t(4,3), edge_t(4,5),
        edge_t(5,1), edge_t(5,4),
        edge_t(6,1), edge_t(6,4),
        edge_t(7,1), edge_t(7,4),
        edge_t(8,1), edge_t(8,4),
        edge_t(9,4),
        edge_t(10,4),
    };
    int num_edges = sizeof(edge_array)/sizeof(edge_array[0]);

    graph_t g(edge_array, edge_array + num_edges, dim);
    std::vector<double> ranks(num_vertices(g));

    page_rank(g, make_iterator_property_map(ranks.begin(),
              get(boost::vertex_index, g)),
              n_iterations(100), 0.85, dim);

    my_id = process_id(g.process_group());

#if 1
    double sum = 0;
    for (int i=0; i < num_vertices(g); i++)
        sum += ranks[i];

    double full_sum = 0;
    boost::mpi::all_reduce(communicator(g.process_group()),
                       sum, full_sum, std::plus<double>());
#else
    double full_sum = 1.0;
#endif

    for (int i=0; i < dim; i++) {
        vertex_t v = vertex(i, g);
        if (owner(v) == my_id)
            std::cout << i << ": " << (ranks[local(v)]/full_sum) << std::endl;
    }

    return 0;
}
