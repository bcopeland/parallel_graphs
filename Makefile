SRCS=pagerank.cc shortest_paths.cc metis_dist_mod.hpp bfs.cc \
 klpart.c queue_modified.ipp ncuts.pl degree_centrality.pl \
 greedy_max_cut.pl greedy_min_cut.pl make_charts.pl \
 make_parts.pl rand.pl roundrobin.pl undirect.pl \
 list.h

DISTFILES=$(SRCS) Makefile README writeup.pdf wikipedia.graph wikipedia.part
DISTDIR=copeland-final-project
DISTNAME=$(DISTDIR).tar.gz

# Set this if you want to build with a local install of boost
# by default assumes /usr/
# BOOST_ROOT=boost_inst
# LIBS=-L$(BOOST_ROOT)/lib -lboost_graph_parallel -lboost_graph \
#	-lboost_system -lboost_mpi -lboost_date_time -lboost_serialization \
#   -lboost_regex

BOOST_ROOT=/usr
LIBS=-lboost_graph_parallel -lboost_system `pkg-config --libs glib-2.0`

# Set CXX to MPECXX and build MPE2 in order to use MPI log facility
MPE_HOME=mpe2-1.3.0
MPECXX=$(MPE_HOME)/bin/mpecc -mpicc=$(MPICXX) -mpilog

# Set this define if you want to use boost version of
# metis_distribution which is _very_ slow
# DEF=-DUSE_SLOW_METIS

MPICXX=mpicxx
CXX=$(MPICXX)
CXXFLAGS=-Wno-deprecated -g $(DEF) -I$(BOOST_ROOT)/include
CFLAGS=-g `pkg-config --cflags glib-2.0`

all: pagerank shortest_paths bfs klpart

clean:
	$(RM) *.o *.ps *.tar.gz *.log *.aux *.sw? pagerank shortest_paths

%.dvi: %.tex
	latex $<
	latex $<

klpart: klpart.o
	$(CC) -o klpart klpart.o $(LIBS)

pagerank: pagerank.o
	$(CXX) $(CXXFLAGS) -o pagerank pagerank.o $(LIBS)

bfs: bfs.o
	$(CXX) $(CXXFLAGS) -o bfs bfs.o $(LIBS)

shortest_paths: shortest_paths.o
	$(CXX) $(CXXFLAGS) -o shortest_paths shortest_paths.o $(LIBS)

centrality: centrality.o
	$(CXX) $(CXXFLAGS) -o centrality centrality.o $(LIBS)

#writeup.pdf: writeup.dvi
#	dvipdf writeup.dvi

dist: clean
	mkdir -p build/$(DISTDIR) && \
	cp -R $(DISTFILES) build/$(DISTDIR) && \
	cd build && \
	tar czvf ../$(DISTNAME) $(DISTDIR)
	$(RM) -r build

