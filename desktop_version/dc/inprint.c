#include "SDL.h"

#include "inline_font.h" /* Actual font data */

#define CHARACTERS_PER_ROW    16 /* I like 16 x 8 fontsets. */
#define CHARACTERS_PER_COLUMN 8  /* 128 x 1 is another popular format. */

static SDL_Surface *inline_font = NULL;
static SDL_Surface *selected_font = NULL;
void prepare_inline_font(void)
{
	Uint8 *pix_ptr, tmp;
	int i, len, j;

	if (inline_font != NULL) { selected_font = inline_font; return; }

	inline_font = SDL_CreateRGBSurface(0, inline_font_width, inline_font_height, 8, 0, 0, 0, 0);

	/* Get pointer to pixels and array length */
	pix_ptr = (Uint8 *)inline_font->pixels;
	len = inline_font->h * inline_font->w / 8;

	/* Copy */
	for (i = 0; i < len; i++) 
	{
		tmp = (Uint8)inline_font_bits[i];
		for (j = 0; j < 8; j++) 
		{
			Uint8 mask = (0x01 << j);
			pix_ptr[i * 8 + j] = (tmp & mask) >> j;
		}
	}

	selected_font = inline_font;
}
void kill_inline_font(void) { SDL_FreeSurface(inline_font); inline_font = NULL; }
void infont(SDL_Surface *font) 
{
	selected_font = font;
	if (font == NULL) prepare_inline_font();
}
void incolor(Uint32 fore, Uint32 back) /* Colors must be in 0x00RRGGBB format ! */
{
	SDL_Color pal[2];
	pal[0].r = (Uint8)((back & 0x00FF0000) >> 16); 
	pal[0].g = (Uint8)((back & 0x0000FF00) >> 8);
	pal[0].b = (Uint8)((back & 0x000000FF));
	pal[1].r = (Uint8)((fore & 0x00FF0000) >> 16); 
	pal[1].g = (Uint8)((fore & 0x0000FF00) >> 8);
	pal[1].b = (Uint8)((fore & 0x000000FF));
	SDL_SetColors(selected_font, pal, 0, 2);
}
void inprint(SDL_Surface *dst, const char *str, Uint32 x, Uint32 y)
{
	SDL_Rect s_rect;
	SDL_Rect d_rect;

	d_rect.x = x;
	d_rect.y = y;
	s_rect.w = selected_font->w / CHARACTERS_PER_ROW;
	s_rect.h = selected_font->h / CHARACTERS_PER_COLUMN;
	d_rect.w = s_rect.w;
	d_rect.h = s_rect.h;

	for (; *str; str++)
	{
		int id = (int)*str;
#if (CHARACTERS_PER_COLUMN != 1)
		int row = id / CHARACTERS_PER_ROW;
		int col = id % CHARACTERS_PER_ROW;
		s_rect.x = col * s_rect.w;
		s_rect.y = row * s_rect.h;
#else
		s_rect.x = id * s_rect.w;
		s_rect.y = 0;
#endif
		if (id == '\n')
		{
			d_rect.x = x;
			d_rect.y += s_rect.h;
			continue;
		}
		SDL_BlitSurface(selected_font, &s_rect, dst, &d_rect);
		d_rect.x += s_rect.w;
	}
}
SDL_Surface *get_inline_font(void) { return selected_font; }
