#include <cstdio>
#include <iostream>

#include <SDL2/SDL.h>

using namespace std;

double perlin(double, double);
double fade(double);
double lerp(double, double, double);
double dotprod(int, double, double);

static int perm[] = { 151,160,137,91,90,15,
   131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
   190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
   135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
   5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
   223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
   129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

static int p[512] = {0};

int main() {
	printf("Hello, Ken Perlin\n");

	for(int i = 0; i < 256; i++) {
		p[i] = p[i + 256] = perm[i];
	}

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("Fucking fail lmao\n");
		return 0;
	}

	int width = 640;
	int height = 480;
	SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

	int* pixels = new int[width * height];
	memset(pixels, 255, width * height * sizeof(int));

	double resolution = 16.0f;
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			double dy = ((double) y) / height;
			double dx = ((double) x) / height;

			double perlVal = perlin(dx * resolution, dy * resolution);
			//printf("perling val:  %f\n", perlVal);
			perlVal = (perlVal + 1)/2;
			
			//converting the perlin noise val to a grayscale value
			int b = (int)(perlVal * 0xFF);
			int g = b * 0x100;
			int r = b * 0x10000;
			int finalVal = r + g + b;
			pixels[y * width + x] =	finalVal;  //No fucking clue whats going on
		}
	}

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
	SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(int));

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);

	printf("Finished rendering perlin map %d x %d\n", width, height);

	SDL_Delay(10000);
}

double perlin(double x, double y) {
	int uX = (int) x & 255;
	int uY = (int) y & 255;
	double dX = x - (int) x;
	double dY = y - (int) y;

	double fx = fade(dX);
	double fy = fade(dY);

	int aa = p[ p[uX] + uY ];		
	int ab = p[ p[uX] + uY + 1 ];		
	int ba = p[ p[ uX + 1 ] + uY ];		
	int bb = p[ p[ uX + 1 ] + uY + 1 ];		

	double dotaa = dotprod(p[aa], dX, dY);
	double dotab = dotprod(p[ab], dX - 1, dY);
	double dotba = dotprod(p[ba], dX, dY - 1);
	double dotbb = dotprod(p[bb], dX - 1, dY - 1);

	double n1 = lerp(fx, dotaa, dotab);
	double n2 = lerp(fx, dotba, dotbb);

	double result = lerp(fy, n1, n2);
	return result;
}

//Riven's modifications to Perlin's Improved Noise algorithm, slightly modified for 2d
double dotprod(int hash, double x, double y) {
	//We select one of the four vectors formed from the origin to the edges
	//of a unit square, and use the last four bits of hash to pick which one to dot product with
	//the vector (x, y). The multiplication is hardcoded in, because it's just with 1s.
	//The vectors are: (1, 0), (0, 1), (-1, 0), (0, -1)

	int bits = (hash & 0xF) % 4;
	switch(bits) {
		case 0x0: return x;
		case 0x1: return y;
		case 0x2: return -x;
		case 0x3: return -y;
		/*
		case 0x4: return y;
		case 0x5: return x;
		case 0x6: return x;
		case 0x7: return -y;
		case 0x8: return -x;
		case 0x9: return x;
		case 0xA: return -x;
		case 0xB: return y;
		case 0xC: return y;
		case 0xD: return -y;
		case 0xE: return -x;
		case 0xF: return y;
		*/
		default: return 0;	//Should never happen
	}
}

double fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double lerp(double t, double a, double b) {
	//return ((1 - t) * a) + (t * b);
	return a + t * (b - a);
}

