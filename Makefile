all: test

CXX=mpicxx
CXXFLAGS=-Wno-deprecated

LIBS=-lboost_graph_parallel -lboost_system

test: test.o
	$(CXX) $(CXXFLAGS) -o test test.o $(LIBS)
