all: build

build:
	g++ -g *.cpp -o pluto

run: build
	./pluto example.ps

clean:
	rm -f pluto
