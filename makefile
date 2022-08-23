
.PHONY: perlin2D perlin3D

perlin2D: 
	gcc main2d.c  perlinNoise.c map.c -lSDL2 -lm -g -Wall -O2 -o perlin2D.out 
	./perlin2D.out

perlin3D: 
	gcc main3d.c  glad.c -lglfw -lm -lGL -g -Wall -O2 -o perlin3D.out 
	./perlin3D.out

