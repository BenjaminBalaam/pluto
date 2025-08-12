all: build

build:
	g++ -g *.cpp -o pluto-script

run: build
	./pluto-script example.ps

clean:
	rm -f pluto-script