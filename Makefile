all: pagerank

CXX=mpicxx
CXXFLAGS=-Wno-deprecated

LIBS=-lboost_graph_parallel -lboost_system

pagerank: pagerank.o
	$(CXX) $(CXXFLAGS) -o pagerank pagerank.o $(LIBS)
