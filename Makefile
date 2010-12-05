SRCS=pagerank.cc shortest_paths.cc metis_dist_mod.hpp
DISTFILES=$(SRCS) Makefile README
DISTDIR=copeland-final-project
DISTNAME=$(DISTDIR).tar.gz

MPE_HOME=mpe2-1.3.0
MPICXX=mpicxx
MPECXX=$(MPE_HOME)/bin/mpecc -mpicc=$(MPICXX) -mpilog

#DEF=-DUSE_SLOW_METIS
CXX=$(MPICXX)
CXXFLAGS=-Wno-deprecated -g $(DEF)
LIBS=-lboost_graph_parallel -lboost_system

all: pagerank shortest_paths

clean:
	$(RM) *.o *.ps *.tar.gz *.log *.aux *.sw? pagerank shortest_paths

%.dvi: %.tex
	latex $<
	latex $<

pagerank: pagerank.o
	$(CXX) $(CXXFLAGS) -o pagerank pagerank.o $(LIBS)

shortest_paths: shortest_paths.o
	$(CXX) $(CXXFLAGS) -o shortest_paths shortest_paths.o $(LIBS)

#writeup.pdf: writeup.dvi
#	dvipdf writeup.dvi

dist: clean
	mkdir -p build/$(DISTDIR) && \
	cp -R $(DISTFILES) build/$(DISTDIR) && \
	cd build && \
	tar czvf ../$(DISTNAME) $(DISTDIR)
	$(RM) -r build

