#include "GraphicsResources.h"
#include "FileSystemUtils.h"
#include <stdio.h>
#include <stdlib.h>

#include "SDL2_stub.h"

// Used to load PNG data
extern "C"
{
	extern unsigned lodepng_decode24(
		unsigned char** out,
		unsigned* w,
		unsigned* h,
		const unsigned char* in,
		size_t insize
	);
	extern unsigned lodepng_decode32(
		unsigned char** out,
		unsigned* w,
		unsigned* h,
		const unsigned char* in,
		size_t insize
	);
}

SDL_Surface* LoadImage(const char *filename, bool noBlend = true, bool noAlpha = false, 
    Uint8 key_r = 0x00, Uint8 key_g = 0x00, Uint8 key_b = 0x00)
//SDL_Surface* LoadImage(const char *filename, bool noBlend = true, bool noAlpha = false)
{

  // Using color keys
  //noAlpha = true;

	//Temporary storage for the image that's loaded
	SDL_Surface* loadedImage = NULL;
	//The optimized image that will be used
	SDL_Surface* optimizedImage = NULL;

	unsigned char *data;
	unsigned int width, height;

	unsigned char *fileIn = NULL;
	size_t length = 0;
	FILESYSTEM_loadFileToMemory(filename, &fileIn, &length);
	if (noAlpha)
	{
		lodepng_decode24(&data, &width, &height, fileIn, length);
	}
	else
	{
		lodepng_decode32(&data, &width, &height, fileIn, length);
	}
	FILESYSTEM_freeMemory(&fileIn);

  // Change every pixel with alpha with the color key
  if (noAlpha == false) {
    for (int i = 0; i < width*height*4; i += 4) {
      if (data[i+3] < 0xFF) {
        // Alpha
        data[i+0] = key_r;
        data[i+1] = key_g;
        data[i+2] = key_b;
      }
    }
  }
  
	loadedImage = SDL_CreateRGBSurfaceFrom(
		data,
		width,
		height,
		noAlpha ? 24 : 32,    
		width * (noAlpha ? 3 : 4),
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		noAlpha ? 0x00000000 : 0xFF000000
	);
	
  if (loadedImage != NULL)
	{ 
    // GUSARBA: Helper Surface
    SDL_Surface* optSurface = SDL_CreateRGBSurface(
      0,
      0,
      0,
#ifndef DREAMCAST
      16,
      0xF800,
      0x07E0,
      0x001F,
      0
    );
#else
      16,
      0xF800,
      0x07E0,
      0x001F,
      0
    );
#endif      
    
		//optimizedImage = SDL_ConvertSurfaceFormat(    
    optimizedImage = SDL_ConvertSurface(
      loadedImage,
      //SDL_PIXELFORMAT_ABGR8888, // FIXME: Format? -flibit
      optSurface->format,
			0
		);
    

    //SDL_SetAlpha(loadedImage, SDL_SRCALPHA, 255);
    //optimizedImage = SDL_DisplayFormatAlpha(loadedImage);
    //optimizedImage = SDL_DisplayFormat(loadedImage);

    SDL_FreeSurface(optSurface);
		SDL_FreeSurface( loadedImage );
		free(data);    
    SDL_SetSurfaceBlendMode(optimizedImage, SDL_BLENDMODE_NONE);    
		if (noBlend)
		{      
			SDL_SetSurfaceBlendMode(optimizedImage, SDL_BLENDMODE_BLEND);            
		}
    printf("LoadImage: %s loaded (%dx%d)\n", filename, width, height);
    //SDL_SetAlpha(optimizedImage, SDL_RLEACCEL, 255);
    SDL_SetColorKey(optimizedImage, SDL_RLEACCEL, SDL_MapRGB(optimizedImage->format, 0, 0, 0));
		return optimizedImage;    
	}
	else
	{
		fprintf(stderr,"Image not found: %s\n", filename);
    printf("LoadImage: COULD NOT LOAD IMAGE %s\n", filename);
		SDL_assert(0 && "Image not found! See stderr.");
		return NULL;
	}
}

void GraphicsResources::init(void)
{
	im_tiles =		LoadImage("graphics/tiles.png");
	im_tiles2 =		LoadImage("graphics/tiles2.png");
	im_tiles3 =		LoadImage("graphics/tiles3.png");
	im_entcolours =		LoadImage("graphics/entcolours.png");
	im_sprites =		LoadImage("graphics/sprites.png");
	im_flipsprites =	LoadImage("graphics/flipsprites.png");
	im_bfont =		LoadImage("graphics/font.png");
	//im_bfontmask =		LoadImage("graphics/fontmask.png");
  im_bfontmask = LoadImage("graphics/fontmask.png", true, false, 0xFF, 0x00, 0xFF);
	im_teleporter =		LoadImage("graphics/teleporter.png");

	im_image0 =		LoadImage("graphics/levelcomplete.png", false);
	im_image1 =		LoadImage("graphics/minimap.png", true, true);
	im_image2 =		LoadImage("graphics/covered.png", true, true);
	im_image3 =		LoadImage("graphics/elephant.png");
	im_image4 =		LoadImage("graphics/gamecomplete.png", false);
	im_image5 =		LoadImage("graphics/fliplevelcomplete.png", false);
	im_image6 =		LoadImage("graphics/flipgamecomplete.png", false);
	im_image7 =		LoadImage("graphics/site.png", false);
	im_image8 =		LoadImage("graphics/site2.png");
	im_image9 =		LoadImage("graphics/site3.png");
	im_image10 =		LoadImage("graphics/ending.png");
	im_image11 =		LoadImage("graphics/site4.png");
	im_image12 =		LoadImage("graphics/minimap.png");
}


GraphicsResources::~GraphicsResources(void)
{
}
