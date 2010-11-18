all: pagerank shortest_paths

CXX=mpicxx
CXXFLAGS=-Wno-deprecated

LIBS=-lboost_graph_parallel -lboost_system

pagerank: pagerank.o
	$(CXX) $(CXXFLAGS) -o pagerank pagerank.o $(LIBS)

shortest_paths: shortest_paths.o
	$(CXX) $(CXXFLAGS) -o shortest_paths shortest_paths.o $(LIBS)
