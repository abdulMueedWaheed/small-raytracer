all:
	gcc rayTracing.c -o output `sdl2-config --cflags --libs` -lm
