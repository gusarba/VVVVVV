#include "Screen.h"

#include "FileSystemUtils.h"
#include "GraphicsUtil.h"

#include <stdlib.h>

#include "SDL2_stub.h"

// GUSARBA: Uncomment this #include statement if you are using Chui's SDL port
//#include <SDL_dreamcast.h>

extern "C"
{
#include "SDL_inprint.h"
}

static Uint32 update_time = 0;

// Used to create the window icon
extern "C"
{
	extern unsigned lodepng_decode24(
		unsigned char** out,
		unsigned* w,
		unsigned* h,
		const unsigned char* in,
		size_t insize
	);
}

Screen::Screen()
{
	m_window = NULL;
	//m_renderer = NULL;
	//m_screenTexture = NULL;
	m_screen = NULL;
	isWindowed = true;
	stretchMode = 0;
	isFiltered = false;
	filterSubrect.x = 1;
	filterSubrect.y = 1;
	filterSubrect.w = 318;
	filterSubrect.h = 238;
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

  /*
	// Uncomment this next line when you need to debug -flibit
	// SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE);
	SDL_CreateWindowAndRenderer(
		640,
		480,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE,
		&m_window,
		&m_renderer
	);
	SDL_SetWindowTitle(m_window, "VVVVVV");
  */
  
	unsigned char *fileIn = NULL;
	size_t length = 0;
	unsigned char *data;
	unsigned int width, height;
	FILESYSTEM_loadFileToMemory("VVVVVV.png", &fileIn, &length);
	lodepng_decode24(&data, &width, &height, fileIn, length);  
	FILESYSTEM_freeMemory(&fileIn);
	SDL_Surface *icon = SDL_CreateRGBSurfaceFrom(  
		data,
		width,
		height,
		24,
		width * 3,
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		0x00000000
	);
	//SDL_SetWindowIcon(m_window, icon);
  SDL_WM_SetIcon(icon, NULL);
	SDL_FreeSurface(icon);
	free(data);

  // GUSARBA: Uncomment these if you are using Chui's SDL port
  /*
  SDL_DC_ShowAskHz(SDL_FALSE);
  SDL_DC_Default60Hz(SDL_TRUE);
  SDL_DC_VerticalWait(SDL_FALSE);
  SDL_DC_SetVideoDriver(SDL_DC_DIRECT_VIDEO);
  */  

  // SDL1.2 Migration
  SDL_WM_SetCaption("VVVVVV", "VVVVVV");
  m_window = SDL_SetVideoMode(320, 240, 16, SDL_HWSURFACE | SDL_DOUBLEBUF);  

	// FIXME: This surface should be the actual backbuffer! -flibit
	m_screen = SDL_CreateRGBSurface(
		0,
		320,
		240,	
    16,
    0x00000F00,
    0x000000F0,
    0x0000000F,
    0x0000F000
	);

  SDL_SetAlpha(m_screen, 0, 255);

  m_screen = m_window;

  /*
	m_screenTexture = SDL_CreateTexture(
		m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		320,
		240
	);
  */

	badSignalEffect = false;
}

void Screen::ResizeScreen(int x, int y)
{
	static int resX = 320;
	static int resY = 240;
	if (x != -1 && y != -1)
	{
		// This is a user resize!
		resX = x;
		resY = y;
	}

	if(!isWindowed)
	{
		//int result = SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    int result = SDL_SetWindowFullscreen(m_screen, SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (result != 0)
		{
			printf("Error: could not set the game to fullscreen mode: %s\n", SDL_GetError());
			return;
		}
	}
	else
	{
		//int result = SDL_SetWindowFullscreen(m_window, 0);
    int result = SDL_SetWindowFullscreen(m_screen, SDL_WINDOW_FULLSCREEN_DESKTOP);
		if (result != 0)
		{
			printf("Error: could not set the game to windowed mode: %s\n", SDL_GetError());
			return;
		}
		if (x != -1 && y != -1)
		{
			//SDL_SetWindowSize(m_window, resX, resY);
      //SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

      // GUSARBA: Disabled at the moment 
      //m_window = SDL_SetVideoMode(resX, resY, 0, SDL_SWSURFACE | SDL_RESIZABLE);
      //m_screen = m_window;
		}
	}
  // GUSARBA: Disabled at the moment
  /*
	if (stretchMode == 1)
	{
		int winX, winY;
		SDL_GetWindowSize(m_window, &winX, &winY);
		int result = SDL_RenderSetLogicalSize(m_renderer, winX, winY);
		if (result != 0)
		{
			printf("Error: could not set logical size: %s\n", SDL_GetError());
			return;
		}
		result = SDL_RenderSetIntegerScale(m_renderer, SDL_FALSE);
		if (result != 0)
		{
			printf("Error: could not set scale: %s\n", SDL_GetError());
			return;
		}
	}
	else
	{
		SDL_RenderSetLogicalSize(m_renderer, 320, 240);
		int result = SDL_RenderSetIntegerScale(m_renderer, (SDL_bool) (stretchMode == 2));
		if (result != 0)
		{
			printf("Error: could not set scale: %s\n", SDL_GetError());
			return;
		}
	}
	SDL_ShowWindow(m_window);
  */
}

void Screen::GetWindowSize(int* x, int* y)
{
	SDL_GetWindowSize(m_window, x, y);
}

void Screen::UpdateScreen(SDL_Surface* buffer, SDL_Rect* rect )
{
  Uint32 func_time = SDL_GetTicks();

	if((buffer == NULL) && (m_screen == NULL) )
	{
		return;
	}

	if(badSignalEffect)
	{
		buffer = ApplyFilter(buffer);
	}


	FillRect(m_screen, 0x000);
	BlitSurfaceStandard(buffer,NULL,m_screen,rect);

	if(badSignalEffect)
	{
		SDL_FreeSurface(buffer);
	}

  update_time += (SDL_GetTicks() - func_time);
}

const SDL_PixelFormat* Screen::GetFormat()
{
	return m_screen->format;  
}

void Screen::FlipScreen()
{
  /*
	SDL_UpdateTexture(
		m_screenTexture,
		NULL,
		m_screen->pixels,
		m_screen->pitch
	);
	SDL_RenderCopy(
		m_renderer,
		m_screenTexture,
		isFiltered ? &filterSubrect : NULL,
		NULL
	);
	SDL_RenderPresent(m_renderer);
	SDL_RenderClear(m_renderer);
  */  

  Uint32 flip_time = 0;
  Uint32 rect_time = 0;
  static char tmpstats[256] = {0};

  // GUSARBA: Disabled at the moment
  /*  
  if (isFiltered) {
    SDL_BlitSurface(m_screen, &filterSubrect, m_window, NULL);
  } else {
    SDL_BlitSurface(m_screen, NULL, m_window, NULL);    
  } 
  */
 
  // GUSARBA: Uncomment these to get debug performance stats 
  /*
  prepare_inline_font();
  incolor(0xFF0000, 0x333333);
  inprint(m_window, tmpstats, 10, 10);
  */

  flip_time = SDL_GetTicks();

  //SDL_UpdateRect(m_window, 0, 0, 0, 0);
  SDL_Flip(m_window);
 
  flip_time = SDL_GetTicks() - flip_time;
  
  rect_time = SDL_GetTicks();

  //SDL_FillRect(m_screen, NULL, 0x00000000);
  //SDL_FillRect(m_window, NULL, 0x00000000);
  
  rect_time = SDL_GetTicks() - rect_time;

  static Uint32 frame_time = SDL_GetTicks();
  static Uint32 prev_time = SDL_GetTicks();

  frame_time = SDL_GetTicks() - prev_time;
  prev_time = SDL_GetTicks();

  static Uint32 t = 0;
  ++t;
  if (t == 30) {
    t = 0;
    sprintf(tmpstats, "ft u bS bC bSn bNn %d %d %d %d %d %d\0", frame_time, update_time, GraphicsStats::BSS_time, GraphicsStats::BSC_time, GraphicsStats::BSS_numcalls, GraphicsStats::BSC_numcalls);    
  }
  GraphicsStats::BSS_time = 0;
  GraphicsStats::BSC_time = 0;
  GraphicsStats::BSS_numcalls = 0;
  GraphicsStats::BSC_numcalls = 0;
  update_time = 0;
}

void Screen::toggleFullScreen()
{
	isWindowed = !isWindowed;
	ResizeScreen(-1, -1);
}

void Screen::toggleStretchMode()
{
	stretchMode = (stretchMode + 1) % 3;
	ResizeScreen(-1, -1);
}

void Screen::toggleLinearFilter()
{
	isFiltered = !isFiltered;
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, isFiltered ? "linear" : "nearest");
  // GUSARBA: Disabled at the moment
  /*
	SDL_DestroyTexture(m_screenTexture);
	m_screenTexture = SDL_CreateTexture(
		m_renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		320,
		240
	);
  */
}
