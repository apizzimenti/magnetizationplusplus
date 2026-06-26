
# Check whether we need tbb.
OS := $(shell uname)
tbb =

ifeq ($(OS),Linux)
	tbb = -ltbb
endif

.PHONY: clean build

reset-simulations: clean
	$(RM) current.timestamps.txt
	$(RM) -rf output/tape/*
	$(RM) completed.txt

clean:
	$(RM) simulate *.o

build: clean
	g++-15 simulate.cpp `pkg-config --libs --cflags ATEAMS++` $(tbb) -o simulate

test: reset-simulations
	@echo building...
	@echo
	@make build
	@echo
	@echo ... done. testing simulation...
	@echo
	@./simulate 4 4 2 TEST
	@echo
	@echo ... done.


simulate: build reset-simulations
	@./simulation.manager.megltower.sh

