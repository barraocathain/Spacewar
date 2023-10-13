// =========================================
// | Spacewar-Graphics.h                   |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef SPACEWAR_GRAPHICS_H
#define SPACEWAR_GRAPHICS_H

#include <stdint.h>
#include <SDL2/SDL.h>

typedef struct SpacewarTitlescreen
{
	SDL_Window * window;
	SDL_Renderer * renderer;
	uint16_t xScroll, titleAlpha, textAlpha;
	SDL_Texture * titleTexture, * textTexture, * starfieldTexture;
	SDL_Rect * titleRectangle, * textRectangle, * starfieldRectangle;
} SpacewarTitlescreen;

typedef struct SpacewarMenuscreen
{
	SDL_Window * window;
	SDL_Renderer * renderer;
	uint16_t xScroll;
	SDL_Texture * starfieldTexture;	
} SpacewarMenuscreen;

SpacewarTitlescreen prepareTitleScreen(SDL_Window * window, SDL_Renderer * renderer,
									   char * starfieldTexturePath, char * logoTexturePath,
									   TTF_Font * font, char * text);

SpacewarMenuscreen prepareMenuscreenFromTitle(SpacewarTitlescreen * titlescreen);

void drawTitleScreen(SpacewarTitlescreen * titlescreen);

void drawMenuScreen(SpacewarMenuscreen * menuscreen);

#endif
// =========================================================
// | End of Spacewar-Graphics.h, copyright notice follows. |
// =========================================================

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
