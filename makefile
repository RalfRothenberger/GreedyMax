CC=g++
SNAPDIR=/usr/lib/snap

all:
	make greedy; make random; make ilp;

greedy:
	$(CC) -std=c++17 -o greedy MaxCutGreedy.cpp $(SNAPDIR)/snap-core/Snap.o -I$(SNAPDIR)/snap-core -I$(SNAPDIR)/snap-adv -I$(SNAPDIR)/glib-core -lstdc++fs -fopenmp
	
random:
	$(CC) -std=c++17 -o random MaxCutRandomGreedy.cpp $(SNAPDIR)/snap-core/Snap.o -I$(SNAPDIR)/snap-core -I$(SNAPDIR)/snap-adv -I$(SNAPDIR)/glib-core -lstdc++fs -fopenmp
	
ilp:
	$(CC) -std=c++17 -o ilp MaxCutILP.cpp $(SNAPDIR)/snap-core/Snap.o -I$(SNAPDIR)/snap-core -I$(SNAPDIR)/snap-adv -I$(SNAPDIR)/glib-core -I$(GUROBI_HOME)/include -L$(GUROBI_HOME)/lib -lgurobi_g++5.2 -lgurobi80 -lstdc++fs -fopenmp
