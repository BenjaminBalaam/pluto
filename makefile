all: clean build run

build:
	g++ *.cpp -o main

run:
	./main example.ps

clean:
	rm -f main