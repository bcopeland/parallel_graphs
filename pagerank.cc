#include <boost/graph/use_mpi.hpp>
#include <boost/graph/distributed/page_rank.hpp>
#include <boost/graph/distributed/adjacency_list.hpp>
#include <boost/graph/distributed/mpi_process_group.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/metis.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <iostream>
#include <stdlib.h>

using namespace boost;
using boost::graph::distributed::mpi_process_group;
using boost::posix_time::ptime;

using namespace boost::graph;
typedef adjacency_list<vecS, distributedS<mpi_process_group, vecS>,
                       directedS> graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef std::pair<int, int> edge_t;
typedef property_map<vertex_t, float> rank_map_t;

struct completion_test
{
    completion_test(std::size_t n) : n(n) {}

    template<typename RankMap, typename Graph>
    bool operator()(const RankMap& map, const Graph& graph)
    {
        int my_id = process_id(graph.process_group());
        return n-- == 0;
    }
 private:
    std::size_t n;
};

int main(int argc, char *argv[])
{
    int i;
    int my_id;

    mpi::environment env(argc, argv);

    std::ifstream in_graph(argv[1]);
    metis_reader reader(in_graph);
    mpi_process_group pg;

    int dim = reader.num_vertices();
    std::ifstream in_partitions(argv[2]);
    metis_distribution dist(in_partitions, process_id(pg));
    graph_t g(reader.begin(), reader.end(), dim, pg, dist);


    my_id = process_id(g.process_group());

    ptime load_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
        std::cerr << "graph loaded " << load_time << std::endl;

    std::vector<double> ranks(num_vertices(g));

    ptime start_time(boost::posix_time::microsec_clock::universal_time());
    page_rank(g, make_iterator_property_map(ranks.begin(),
              get(boost::vertex_index, g)),
              completion_test(10), 0.85, dim);

#if 0
    double sum = 0;
    for (int i=0; i < num_vertices(g); i++)
        sum += ranks[i];

    double full_sum = 0;
    boost::mpi::all_reduce(communicator(g.process_group()),
                       sum, full_sum, std::plus<double>());
#else
    double full_sum = 1.0;
#endif

    synchronize(g.process_group());

    ptime end_time(boost::posix_time::microsec_clock::universal_time());
    if (my_id == 0)
         std::cout << "completed PR in " << (end_time - start_time) << std::endl;

    for (int i=0; i < dim; i++) {
        vertex_t v = vertex(i, g);
        if (owner(v) == my_id)
            std::cout << i << ": " << (ranks[local(v)]/full_sum) << " " <<
                my_id << std::endl;
    }

    return 0;
}
