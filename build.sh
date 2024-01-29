#!/bin/sh

set -xe

mkdir -p bin
# gcc -Wall -Wextra -o bin/voronoi-ppm src/main_ppm.c
gcc -Wall -Wextra -o bin/voronoi-opengl src/main_opengl.c -lglfw -lGL -lm