all:
	gcc -o main main.c parser.c raycaster.c -lm

run:
	./main 500 500 input.json output.ppm
