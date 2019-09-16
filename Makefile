OBJ_NAME_2 = bin/perlin

all : 
	g++ perlin3d.cpp -w -lSDL2 -o $(OBJ_NAME_2)
