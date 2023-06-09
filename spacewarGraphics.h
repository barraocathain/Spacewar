#ifndef SPACEWAR_GRAPHICS_H
#define SPACEWAR_GRAPHICS_H
#include <SDL2/SDL.h>

typedef struct SpacewarTitlescreen
{
	SDL_Window * window;
	SDL_Renderer * renderer;
	uint16_t xScroll, titleAlpha, textAlpha;
	SDL_Texture * titleTexture, * textTexture, * starfieldTexture;
	SDL_Rect * titleRectangle, * textRectangle, * starfieldRectangle;
} SpacewarTitlescreen;

SpacewarTitlescreen prepareTitleScreen(SDL_Window * window, SDL_Renderer * renderer,
									   char * starfieldTexturePath, char * logoTexturePath,
									   TTF_Font * font, char * text);

void drawTitleScreen(SpacewarTitlescreen * titlescreen);

static inline void DrawCircle(SDL_Renderer * renderer, int32_t centreX, int32_t centreY, int32_t radius)
{
	const int32_t diameter = (radius * 2);

	int32_t x = (radius - 1); 
	int32_t y = 0;
	int32_t tx = 1;
	int32_t ty = 1;
	int32_t error = (tx - diameter);

	while (x >= y)
	{
		//  Each of the following renders an octant of the circle
		SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
		SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
		SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

		if (error <= 0)
		{
			++y;
			error += ty;
			ty += 2;
		}

		if (error > 0)
		{
			--x;
			tx += 2;
			error += (tx - diameter);
		}
	}
}

#endif
