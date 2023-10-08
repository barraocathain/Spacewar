#include <stdint.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "Spacewar-Graphics.h"

SpacewarTitlescreen prepareTitleScreen(SDL_Window * window, SDL_Renderer * renderer,
									   char * starfieldTexturePath, char * logoTexturePath,
									   TTF_Font * font, char * text)
{
	SpacewarTitlescreen newTitleScreen;

	// Basic state:
	newTitleScreen.xScroll = 0;
	newTitleScreen.textAlpha = 0;
	newTitleScreen.titleAlpha = 0;

	// Requirements for drawing:
	newTitleScreen.window = window;
	newTitleScreen.renderer = renderer;

	// Textures:
	newTitleScreen.titleTexture = IMG_LoadTexture(renderer, logoTexturePath);
	newTitleScreen.starfieldTexture = IMG_LoadTexture(renderer, starfieldTexturePath);

	// Text message:
	SDL_Color white = {255, 255, 255};
	SDL_Surface * fontSurface = TTF_RenderText_Blended(font, text, white);
	newTitleScreen.textTexture = SDL_CreateTextureFromSurface(renderer, fontSurface);
	SDL_FreeSurface(fontSurface);

	// Rects:
	newTitleScreen.titleRectangle = calloc(1, sizeof(SDL_Rect));
	newTitleScreen.textRectangle = calloc(1, sizeof(SDL_Rect));
	newTitleScreen.starfieldRectangle = calloc(1, sizeof(SDL_Rect));

	// Set the rects to the size of the textures:
	SDL_QueryTexture(newTitleScreen.textTexture, NULL, NULL, NULL, &newTitleScreen.textRectangle->h);
	SDL_QueryTexture(newTitleScreen.textTexture, NULL, NULL, &newTitleScreen.textRectangle->w, NULL);
	SDL_QueryTexture(newTitleScreen.titleTexture, NULL, NULL, NULL, &newTitleScreen.titleRectangle->h);
	SDL_QueryTexture(newTitleScreen.titleTexture, NULL, NULL, &newTitleScreen.titleRectangle->w, NULL);
	SDL_QueryTexture(newTitleScreen.starfieldTexture, NULL, NULL, NULL, &newTitleScreen.starfieldRectangle->h);
	SDL_QueryTexture(newTitleScreen.starfieldTexture, NULL, NULL, &newTitleScreen.starfieldRectangle->w, NULL);

	return newTitleScreen;
}

void drawTitleScreen(SpacewarTitlescreen * titlescreen)
{
	// Get the current size of the window:
	int width = 0, height = 0;
	SDL_GetWindowSize(titlescreen->window, &width, &height);

	// Position the elements on-screen:
	titlescreen->titleRectangle->x = (width/2) - (titlescreen->titleRectangle->w / 2); 
	titlescreen->titleRectangle->y = (height/2) - titlescreen->titleRectangle->h;

	titlescreen->textRectangle->x = (width/2) - (titlescreen->textRectangle->w / 2); 
	titlescreen->textRectangle->y = (height/2) + (titlescreen->textRectangle->h * 2);

	// Set the renderer colour to black and clear the screen:
	SDL_SetRenderDrawColor(titlescreen->renderer, 0, 0, 0, 255);
	SDL_RenderClear(titlescreen->renderer);

	// Set the correct position to begin the starfield, and scroll it back for the next frame:
	titlescreen->starfieldRectangle->x = 0 - titlescreen->xScroll++;
	titlescreen->starfieldRectangle->y = 0;
	
	// Draw the starfield by tiling the starfield texture:
	while (titlescreen->starfieldRectangle->x <= (width + titlescreen->starfieldRectangle->w))
	{
		// Go down, covering a column of the screen:
		while(titlescreen->starfieldRectangle->y <= (height + titlescreen->starfieldRectangle->h))
		{
			SDL_RenderCopy(titlescreen->renderer, titlescreen->starfieldTexture, NULL,
						   titlescreen->starfieldRectangle);
			titlescreen->starfieldRectangle->y += titlescreen->starfieldRectangle->h;			
		}
		
		// Back to the top, move over one texture width:
		titlescreen->starfieldRectangle->y = 0;
		titlescreen->starfieldRectangle->x += titlescreen->starfieldRectangle->w;
	}

	// Reset the xScroll if it goes farther than a texture width away:
	if (titlescreen->xScroll == titlescreen->starfieldRectangle->w + 1)
	{
		titlescreen->xScroll = 0;
	}

	// Set the opacity of the logo so we can fade it in:
	if (titlescreen->titleAlpha < 254)
	{
		titlescreen->titleAlpha += 10;
	}
	if (titlescreen->titleAlpha >= 254)
	{
		titlescreen->titleAlpha = 254;
	}
	SDL_SetTextureAlphaMod(titlescreen->titleTexture, titlescreen->titleAlpha);

	// Set the opacity of the text so we can fade it in after we fade in the logo:
	if (titlescreen->textAlpha < 254 && titlescreen->titleAlpha == 254)
	{
		titlescreen->textAlpha += 10;
	}
	if (titlescreen->textAlpha >= 254)
	{
		titlescreen->textAlpha = 254;
	}
	SDL_SetTextureAlphaMod(titlescreen->textTexture, titlescreen->textAlpha);
	
	// Display the logo and text:
	SDL_RenderCopy(titlescreen->renderer, titlescreen->titleTexture,  NULL,  titlescreen->titleRectangle);
	SDL_RenderCopy(titlescreen->renderer, titlescreen->textTexture,  NULL,  titlescreen->textRectangle);
	
	// Display to the renderer:
	SDL_RenderPresent(titlescreen->renderer);
}

void drawMenuScreen(SpacewarMenuscreen * menuscreen)
{
	// Get the current size of the window:
	int width = 0, height = 0;
	SDL_GetWindowSize(titlescreen->window, &width, &height);

	// Set the renderer colour to black and clear the screen:
	SDL_SetRenderDrawColor(titlescreen->renderer, 0, 0, 0, 255);
	SDL_RenderClear(titlescreen->renderer);

	// Set the correct position to begin the starfield, and scroll it back for the next frame:
	titlescreen->starfieldRectangle->x = 0 - titlescreen->xScroll++;
	titlescreen->starfieldRectangle->y = 0;
	
	// Draw the starfield by tiling the starfield texture:
	while (titlescreen->starfieldRectangle->x <= (width + titlescreen->starfieldRectangle->w))
	{
		// Go down, covering a column of the screen:
		while(titlescreen->starfieldRectangle->y <= (height + titlescreen->starfieldRectangle->h))
		{
			SDL_RenderCopy(titlescreen->renderer, titlescreen->starfieldTexture, NULL,
						   titlescreen->starfieldRectangle);
			titlescreen->starfieldRectangle->y += titlescreen->starfieldRectangle->h;			
		}
		
		// Back to the top, move over one texture width:
		titlescreen->starfieldRectangle->y = 0;
		titlescreen->starfieldRectangle->x += titlescreen->starfieldRectangle->w;
	}
	
	// Display to the renderer:
	SDL_RenderPresent(titlescreen->renderer);
}

