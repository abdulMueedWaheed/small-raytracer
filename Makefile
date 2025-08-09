all:
	gcc rayTracing.c -o rt `sdl2-config --cflags --libs` -lm
