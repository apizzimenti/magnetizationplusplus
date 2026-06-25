
.PHONY: clean build

reset-simulations: clean
	$(RM) current.timestamps.txt
	$(RM) -rf output/tape/*
	$(RM) completed.txt

clean:
	$(RM) simulate *.o

build-macos: clean
	g++-15 simulate.cpp `pkg-config --libs --cflags ATEAMS++` -o simulate

build-linux: clean
	g++-15 simulate.cpp `pkg-config --libs --cflags ATEAMS++` -ltbb -o simulate

test: reset-simulations
	@echo building...
	@echo
	@make build-macos
	@echo
	@echo ... done. testing simulation...
	@echo
	@./simulate 4 4 2 TEST
	@echo
	@echo ... done.


simulate: build-linux reset-simulations
	@./simulation.manager.megltower.sh

