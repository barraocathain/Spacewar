// =========================================
// | Spacewar-Client.c                     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include "Spacewar-Messages.h"
#include "Spacewar-Graphics.h"
#include "Spacewar-Server.h"

const char * messageStrings[] = {"HELLO", "GOODBYE", "PING", "PONG", "SECRET"};

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
	bool runServer = false;
	
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

		// Check if Space was pressed:
		if(keyboardState[SDL_SCANCODE_SPACE] == 1 && !runServer)
		{		
			runServer = true;
			printf("Running server!\n");
		}
		
		// Draw the title screen:
		drawTitleScreen(&titlescreen);

		// Delay enough so that we run at 60 frames in the menu:
		SDL_Delay(1000 / 60);
	}

	if (runServer)
	{
		SpacewarServerConfiguration serverConfig = {5200};
		pthread_t serverThread;
		pthread_create(&serverThread, NULL, runSpacewarServer, &serverConfig);
		sleep(1);
	}
	
	// Give me a socket, and make sure it's working:
	int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == -1)
	{
		printf("Socket creation failed.\n");
		exit(EXIT_FAILURE);
	}

	// Create an address struct to point at the server:
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(5200);
  
	// Connect to the server:
	if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) != 0)
	{
		fprintf(stderr, "Connecting to the server failed.\n");
		exit(0);
	}

	printf("Connected.\n");

	bool playerNumberSet, secretKeySet;
	uint32_t playerNumber, secretKey;
	SpacewarMessage message;

	while (!playerNumberSet || !secretKeySet)
	{
		recv(serverSocket, &message, sizeof(SpacewarMessage), 0);
		switch (message.type)
		{
			case 0:
			{
				playerNumberSet = true;
				playerNumber = message.content;
				break;
			}
			case 4:
			{
				secretKeySet = true;
				secretKey = message.content;
			}
		}			
	}

	printf("Player Number: %u\n"
		   "Secret Key: %u\n", playerNumber, secretKey);
	
	// Spawn network input thread:
	
	// Spawn client-side-prediction thread:
	if (!runServer)
	{
		pthread_t clientSidePredictionThread;
	}
	
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
// compile-command: "gcc `sdl2-config --libs --cflags` Spacewar-Client.c Spacewar-Graphics.c Spacewar-Server.c -lSDL2_image -lSDL2_ttf -lm -o 'Spacewar-Client'"
// End:
