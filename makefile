all:
	gcc -o main parser.c raycaster.c main.c -lm

run:
	./main 500 500 input.json output.ppm

debug:
	gcc parser.c raycaster.c main.c -lm
	gdb a.out
