#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/page_rank.hpp>
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
typedef adjacency_list<vecS, distributedS<mpi_process_group, vecS>,
                       directedS> graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef std::pair<int, int> edge_t;
typedef property_map<vertex_t, float> rank_map_t;

int main(int argc, char *argv[])
{
    int i;
    int dim = 11;
    int my_id;

    mpi::environment env(argc, argv);

    std::ifstream in_graph(argv[1]);
    metis_reader reader(in_graph);
    mpi_process_group pg;

    std::ifstream in_partitions(argv[2]);
    metis_distribution dist(in_partitions, process_id(pg));
    graph_t g(reader.begin(), reader.end(),
            reader.num_vertices(), pg, dist);

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
            std::cout << i << ": " << (ranks[local(v)]/full_sum) << " " <<
                my_id << std::endl;
    }

    return 0;
}
