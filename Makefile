all: run

build:
	g++ -g *.cpp -o main

run: build
	./main example.ps

clean:
	rm -f main