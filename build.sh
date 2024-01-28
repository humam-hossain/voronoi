#!/bin/sh

set -xe

mkdir -p bin
gcc -Wall -Wextra -o bin/voronoi src/main.c