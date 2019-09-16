#include <cstdio>
#include <iostream>
#include <math.h>

#include <SDL2/SDL.h>

using namespace std;

double perlin(double, double, double);
double fade(double);
double lerp(double, double, double);
double grad(int, double, double, double);
double octavePerlin(double, double, double, double, double, double, int);
void generateMap(int width, int height, double z, double persistance, double amplitude, double frequency, int numberOfOctaves, int* pixels) ;
static int perm[] = { 151,160,137,91,90,15, 131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23, 190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
   88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
   77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
   102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196, 135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123, 5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42, 223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9, 129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
   251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
   49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
   138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
};

static int p[512] = {0};

int main(int argc, char** argv) {
	for(int i = 0; i < 256; i++) {
		p[256 + i] = p[i] = perm[i];
	}

	SDL_Window* window = NULL;
	SDL_Renderer* renderer = NULL;
  bool quit = false;

	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL broke :(\n");
		return 0;
	}

  double amplitude = 0;
  int octaves = 0;

  if(argc > 3) {
    printf("Too many arguments lol\n");
    return 0;
  }

  if(argc < 2) {
    amplitude = 1.0;
    octaves = 3;
  }
  else {
    amplitude = atof(argv[1]);
    octaves = atoi(argv[2]);
  }

  //Some defaults:
  double persistence = 0.5; 
  double zVal = 4;
	int width = 512;
	int height = 512;

	SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
  SDL_RenderPresent(renderer);

  int* pixels = new int[width * height];

  while(!quit) {
    double inputFreq = 0;

    cout << "Enter the frequency: " << endl;
    cin >> inputFreq; 

    if(inputFreq == 0.0f) {
      quit = true; 
      break;
    }

    generateMap(width, height, zVal, persistence, amplitude, inputFreq, octaves, pixels);
    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(int));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);  
    SDL_RenderPresent(renderer);
    
	  printf("Finished rendering perlin map for f = %f @ (%d, %d)px\n", inputFreq, width, height);
  }

  delete[] pixels;
  SDL_DestroyTexture(texture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  printf("Program ended\n");
  SDL_Quit();

  return 0;
}

//The generated map is stored in pixels.
void generateMap(int width, int height, double z, double persistance, double amplitude, double frequency, int numberOfOctaves, int* pixels) {
	memset(pixels, 255, width * height * sizeof(int));

  int green = 0x608038;
  int water = 0x0077be;
  int sand = 0xc2b280;
  int snow = 0xfffafa;
  int rocky = 0x353644;
  int swampy = 0x152b12;

	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			double dx = ((double) x) / height;
			double dy = ((double) y) / width;

      double perlVal = octavePerlin(dx, dy, z, persistance, amplitude, frequency, numberOfOctaves);  

      if(perlVal < -1 || perlVal > 1) {
        printf("Rogue values detected! RUNNNNN\n");
      }
			
			//converting the perlin noise val to a grayscale value
      int finalVal = 0;

      //For grayscale
      int b = (int)(perlVal * 0xFF);
      int g = b * 0x100;
      int r = b * 0x10000;

      if(perlVal < -0.05) finalVal = water;
      //else if(perlVal < -0.05) finalVal = swampy;
      else if(perlVal < 0) finalVal = sand;
      else if(perlVal < 0.4) finalVal = green;
      else if(perlVal < 1.0) finalVal = snow;

      //Generate islands on -1 - 1
      //if(perlVal > 0.3 && perlVal <= 0.4) finalVal = sand;
      //else if(perlVal <= 0.3) finalVal = water;
      //else if(perlVal >= 0.65) finalVal = snow;
      //else finalVal = green;
			pixels[y * width + x] = finalVal;
		}
	}
}

double octavePerlin(double x, double y, double z, double persistance, double startAmp, double startFreq, int numOctaves) {
  if(persistance <= 0 || persistance >= 1) {
    printf("Persistance needs to be 0 < p < 1");
    return 0;
  }

  double amp = startAmp;
  double freq = startFreq;
  double total = 0;
  double maxValue = 0;

  for(int i = 0; i < numOctaves; i++) {
    total += amp * perlin(x * freq, y * freq, z * freq);
    freq *= 2;    //frequency doubles every octave
    maxValue += amp;
    amp *= persistance; //amplitude drops (persistence needs to be 0< persistence < 1)
  }

  return total/maxValue;
}

double perlin(double x, double y, double z) {
     int X = (int)x & 255,                  // FIND UNIT CUBE THAT
          Y = (int)y & 255,                  // CONTAINS POINT.
          Z = (int)z & 255;
      x -= (int) x;                                // FIND RELATIVE X,Y,Z
      y -= (int) y;                                // OF POINT IN CUBE.
      z -= (int) z;
      double u = fade(x),                                // COMPUTE FADE CURVES
             v = fade(y),                                // FOR EACH OF X,Y,Z.
             w = fade(z);
      int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
          B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

  double result = lerp(w, lerp(v, lerp(u, grad(p[AA  ], x  , y  , z   ),  // AND ADD
                                     grad(p[BA  ], x-1, y  , z   )), // BLENDED
                             lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
                                     grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
                     lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
                                     grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
                             lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                                     grad(p[BB+1], x-1, y-1, z-1 ))));
	//return (result + 1) / 2;	//Translate the [-1, 1] range to [0, 1] range
  return result;

}

double grad(int hash, double x, double y, double z) {
	 int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
      double u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
             v = h<4 ? y : h==12||h==14 ? x : z;
      return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

double fade(double t) {
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double lerp(double t, double a, double b) {
  return ((1 - t) * a) + (t * b);
}

