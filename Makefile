all: build

build:
	g++ -g src/*.cpp src/main/main.cpp -o bin/pluto

run: build
	./bin/pluto example.ps

clean:
	rm -f bin/*

build_test:
	g++ -g src/*.cpp tests/*.cpp -o bin/tests

run_test:
	./bin/tests