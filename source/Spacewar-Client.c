// =========================================
// | Spacewar-Client.c                     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>

#include "Spacewar-Graphics.h"

int main(int argc, char ** argv)
{
	// Initialize the SDL library, video, sound, and input:
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL Initialization Error: %s\n", SDL_GetError());
	}

	// Initialize image loading:
	IMG_Init(IMG_INIT_PNG);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

	// Initialize font support:
	TTF_Init();

	// Create an SDL window and rendering context in that window:
	SDL_Window * window = SDL_CreateWindow("SDL_TEST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, 0);
	uint32_t rendererFlags = SDL_RENDERER_ACCELERATED;
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, rendererFlags);
	SDL_SetWindowTitle(window, "Spacewar!");
	SDL_SetWindowResizable(window, SDL_TRUE);

	// Set up keyboard input:
	int keyCount = 0; 
	const uint8_t * keyboardState = SDL_GetKeyboardState(&keyCount);
	
	// Prep the titlescreen struct:
	bool titlescreenInput = false;
	TTF_Font * font = TTF_OpenFont("../Robtronika.ttf", 12);
	SpacewarTitlescreen titlescreen = prepareTitleScreen(window, renderer,
														 "../Images/Starfield.png",
														 "../Images/Title.png", font,
														 "Press Enter or Button 0.");
	// Set up event handling:
	SDL_Event event;
	bool keepRunning = true;
	
	// Display the titlescreen until we get an input:
	while (!titlescreenInput && keepRunning)
	{
		// Update events and input:
		SDL_PumpEvents();
		SDL_GetKeyboardState(&keyCount);

		// Check windowing system events:
		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					keepRunning = false;
					continue;
				}
			}
		}
		
		// Check if Enter was pressed:
		if(keyboardState[SDL_SCANCODE_RETURN] == 1)
		{		
			titlescreenInput = true;
		}

		// Draw the title screen:
		drawTitleScreen(&titlescreen);

		// Delay enough so that we run at 60 frames in the menu:
		SDL_Delay(1000 / 60);
	}

	
	// Connect to server:
	while (keepRunning)
	{
		// Update events and input:
		SDL_PumpEvents();
		SDL_GetKeyboardState(&keyCount);

		// Check windowing system events:
		while (SDL_PollEvent(&event) != 0)
		{
			switch (event.type)
			{
				case SDL_QUIT:
				{
					keepRunning = false;
					continue;
				}
			}
		}
		
		drawMenuScreen(&titlescreen);

		// Delay enough so that we run at 60 frames in the menu:
		SDL_Delay(1000 / 60);
	}
	
	// Spawn network thread:

	// Spawn game thread:

	// Spawn graphics thread:

	return 0;
}
// =======================================================
// | End of Spacewar-Client.c, copyright notice follows. |
// =======================================================

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

// Local Variables:
// compile-command: "gcc `sdl2-config --libs --cflags` Spacewar-Client.c Spacewar-Graphics.c -lSDL2_image -lSDL2_ttf -lm -o 'Spacewar-Client'"
// End:
