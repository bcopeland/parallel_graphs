// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_METIS_DIST_MOD_HPP
#define BOOST_GRAPH_METIS_DIST_MOD_HPP

#ifdef BOOST_GRAPH_METIS_NO_INLINE
#  define BOOST_GRAPH_METIS_INLINE_KEYWORD
#else
#  define BOOST_GRAPH_METIS_INLINE_KEYWORD inline
#endif

#include <string>
#include <iostream>
#include <iterator>
#include <utility>
#include <sstream>
#include <exception>
#include <vector>
#include <algorithm>

namespace boost { namespace graph {

class metis_distribution_mod
{
 public:  
  typedef int process_id_type;
  typedef std::size_t size_type;
  typedef std::vector<process_id_type>::iterator iterator;

  metis_distribution_mod(std::istream& in,
    boost::graph::distributed::mpi_process_group& pg);
  
  size_type block_size(process_id_type id, size_type) const;
  process_id_type operator()(size_type n) const { return vertices[n]; }
  size_type local(size_type n) const;
  size_type global(size_type n) const { return global(my_id, n); }
  size_type global(process_id_type id, size_type n) const;

  iterator begin() { return vertices.begin(); }
  iterator end()   { return vertices.end(); }

 private:
  std::istream& in;
  process_id_type my_id;
  std::vector<process_id_type> vertices;
  std::vector<size_type> local_mapping;
};

metis_distribution_mod::metis_distribution_mod(std::istream& in,
  boost::graph::distributed::mpi_process_group& pg)
  : in(in), my_id(process_id(pg)), 
    vertices(std::istream_iterator<process_id_type>(in),
             std::istream_iterator<process_id_type>())
{
    local_mapping.resize(vertices.size());
    for (int id = 0; id < num_processes(pg); id++)
    {
        size_type count_n = 0;
        int index = 0;
        for (std::vector<process_id_type>::const_iterator i=vertices.begin(); i < vertices.end(); ++i) {
            if (id == *i) {
                local_mapping[index] = count_n++;
            }
            index++;
        }
    }
}


metis_distribution_mod::size_type 
metis_distribution_mod::block_size(process_id_type id, size_type) const
{
  return std::count(vertices.begin(), vertices.end(), id);
}

metis_distribution_mod::size_type metis_distribution_mod::local(size_type n) const
{
  return local_mapping[n];
}

metis_distribution_mod::size_type 
metis_distribution_mod::global(process_id_type id, size_type n) const
{
  std::vector<process_id_type>::const_iterator i = vertices.begin();
  while (*i != id) ++i;

  while (n > 0) {
    do { ++i; } while (*i != id); 
    --n;
  }

  return i - vertices.begin();
}

} } // end namespace boost::graph

#endif // BOOST_GRAPH_METIS_DIST_MOD_HPP
