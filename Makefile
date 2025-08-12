all: build

build:
	g++ -g src/*.cpp -o bin/pluto

run: build
	./bin/pluto example.ps

clean:
	rm -f bin/pluto
