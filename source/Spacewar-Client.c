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
typedef struct SpacewarNetworkConfig
{
	SpacewarClientInput input;
	SpacewarState * state;
} SpacewarNetworkConfig;

void * runNetworkThread (void * parameters)
{
	SpacewarNetworkConfig * configuration = (SpacewarNetworkConfig *)parameters;
	int udpSocket = 0;
	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);

	// Configure a timeout for receiving:
	struct timeval receiveTimeout;
	receiveTimeout.tv_sec = 0;
	receiveTimeout.tv_usec = 1000;
	setsockopt(udpSocket, SOL_SOCKET, SO_RCVTIMEO, &receiveTimeout, sizeof(struct timeval));

	// Point at the server:
	struct sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(5200);

	// A structure to store the most recent state from the network:
	SpacewarState * updatedState = calloc(1, sizeof(SpacewarState));
	
	while (true)
	{
		sendto(udpSocket, &configuration->input, sizeof(SpacewarClientInput), 0,
			   (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
		recvfrom(udpSocket, updatedState, sizeof(SpacewarState), 0, NULL, NULL);
		memcpy(configuration->state, updatedState, sizeof(SpacewarState));
	}
}

int main(int argc, char ** argv)
{
	xyVector upVector = {0, 0.1};
	
	// Initialize the SDL library, video, sound, and input:
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL Initialization Error: %s\n", SDL_GetError());
	}

	// Initialize image loading:
	IMG_Init(IMG_INIT_PNG);
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	// Initialize font support:
	TTF_Init();

	// Create an SDL window and rendering context in that window:
	SDL_Window * window = SDL_CreateWindow("SDL_TEST", SDL_WINDOWPOS_CENTERED,
										   SDL_WINDOWPOS_CENTERED, 640, 360, 0);   
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
	uint8_t playerNumber;
	SpacewarNetworkConfig networkConfiguration;
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
				networkConfiguration.input.playerNumber = message.content;
				break;
			}
			case 4:
			{
				secretKeySet = true;
				networkConfiguration.input.secret = message.content;
			}
		}			
	}

	SpacewarState * state = calloc(1, sizeof(SpacewarState));
	networkConfiguration.state = state;
	
	// Spawn network thread:
	pthread_t networkThread;
	pthread_create(&networkThread, NULL, runNetworkThread, &networkConfiguration);
	
	// Spawn client-side-prediction thread:
	if (!runServer)
	{
		pthread_t clientSidePredictionThread;
	}

	// Load in all of our textures:
	SDL_Texture * blackHoleTexture, * idleTexture, * acceleratingTexture, * clockwiseTexture,
		* anticlockwiseTexture, * currentTexture, * acceleratingTexture2, *blackHolePointerTexture;

	SDL_Rect starfieldRect;
	SDL_Texture * starfieldTexture = IMG_LoadTexture(renderer, "../Images/Starfield.png");
	SDL_QueryTexture(starfieldTexture, NULL, NULL, &starfieldRect.w, &starfieldRect.h);
	
	idleTexture = IMG_LoadTexture(renderer, "../Images/Ship-Idle.png");
	blackHoleTexture = IMG_LoadTexture(renderer, "../Images/Black-Hole.png");
	clockwiseTexture = IMG_LoadTexture(renderer, "../Images/Ship-Clockwise.png");
	acceleratingTexture = IMG_LoadTexture(renderer, "../Images/Ship-Accelerating.png");
	anticlockwiseTexture = IMG_LoadTexture(renderer, "../Images/Ship-Anticlockwise.png");
	blackHolePointerTexture = IMG_LoadTexture(renderer, "../Images/Black-Hole-Pointer.png");
	acceleratingTexture2 = IMG_LoadTexture(renderer, "../Images/Ship-Accelerating-Frame-2.png");
	currentTexture = acceleratingTexture;

	SDL_Rect blackHoleRectangle;
	blackHoleRectangle.x = 0;
	blackHoleRectangle.y = 0;
	SDL_QueryTexture(blackHoleTexture, NULL, NULL,
					 &blackHoleRectangle.w, &blackHoleRectangle.h);


	SDL_Rect blackHolePointerRectangle;
	blackHolePointerRectangle.x = 0;
	blackHolePointerRectangle.y = 0;
	blackHolePointerRectangle.w = 128;
	blackHolePointerRectangle.h = 128;

	SDL_Rect shipRectangles[32];
	for (int index = 0; index < 32; index++)
	{
		shipRectangles[index].w = 32;
		shipRectangles[index].h = 32;
	}

	SDL_Rect rendererSize;
	int width, height;
	while (true)
	{
		SDL_PumpEvents();
		SDL_GetKeyboardState(&keyCount);
		//SDL_GetWindowSize(window, &width, &height);
		
		// Do input:
		networkConfiguration.input.input.turningClockwise = keyboardState[SDL_SCANCODE_RIGHT];
		networkConfiguration.input.input.turningAnticlockwise = keyboardState[SDL_SCANCODE_LEFT];
		networkConfiguration.input.input.accelerating = keyboardState[SDL_SCANCODE_UP];

		if  (keyboardState[SDL_SCANCODE_PAGEUP] == 1)
		{
			float scaleX, scaleY;
			SDL_RenderGetScale(renderer, &scaleX, &scaleY);
			SDL_RenderSetScale(renderer, scaleX + 0.01, scaleY + 0.01);
		}

		if  (keyboardState[SDL_SCANCODE_PAGEDOWN] == 1)
		{
			float scaleX, scaleY;
			SDL_RenderGetScale(renderer, &scaleX, &scaleY);
			SDL_RenderSetScale(renderer, scaleX - 0.01, scaleY - 0.01);
		}

		SDL_RenderGetViewport(renderer, &rendererSize);
		width = rendererSize.w;
		height = rendererSize.h;
		
		// Clear the screen:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);

		// Draw the starfield:
		starfieldRect.x = -900 - (long)state->playerStates[playerNumber].position.xComponent % 800;
		starfieldRect.y = -900 - (long)state->playerStates[playerNumber].position.yComponent % 800;

		while(starfieldRect.x <= (width + 800))
		{
			while(starfieldRect.y <= (height + 800))
			{
				SDL_RenderCopy(renderer, starfieldTexture,  NULL,  &starfieldRect);
 				starfieldRect.y += 800;
			}
			starfieldRect.y = -900 - (long)state->playerStates[playerNumber].position.yComponent % 800;
			starfieldRect.x += 800;
		}

		// Draw the black hole:
		blackHoleRectangle.x = ((long)(0 - state->playerStates[playerNumber].position.xComponent
									   - (blackHoleRectangle.w / 2)) + width/2);
		blackHoleRectangle.y = ((long)(0 - state->playerStates[playerNumber].position.yComponent
									   - (blackHoleRectangle.h / 2)) + height/2);
		
		SDL_RenderCopy(renderer, blackHoleTexture, NULL, &blackHoleRectangle);

		// Draw the black hole directional indicator:
		blackHolePointerRectangle.x = width/2  - (blackHolePointerRectangle.w / 2);
		blackHolePointerRectangle.y = height/2 - (blackHolePointerRectangle.h / 2);
		SDL_RenderCopyEx(renderer, blackHolePointerTexture, NULL, &blackHolePointerRectangle,
						 angleBetweenVectors(&state->playerStates[playerNumber].gravity, &upVector) + 90,
						 NULL, 0);

		// Draw the player's ship:
		shipRectangles[playerNumber].x = (width/2) - 16;
		shipRectangles[playerNumber].y = (height/2) - 16;
		
		SDL_RenderCopyEx(renderer, currentTexture, NULL, &shipRectangles[playerNumber],
						 angleBetweenVectors(&state->playerStates[playerNumber].engine, &upVector) + 90,
						 NULL, 0);

		// Draw every other ship:
		for (int index = 0; index < 32; index++)
		{
			if (index == playerNumber)
			{
				continue;
			}
			if (state->playerStates[playerNumber].inPlay == true)
			{
				shipRectangles[index].x = ((long)(state->playerStates[index].position.xComponent -
										   state->playerStates[playerNumber].position.xComponent) -
										   32 + (width/2));
				shipRectangles[index].y = ((long)(state->playerStates[index].position.yComponent -
												  state->playerStates[playerNumber].position.yComponent) -
										   32 + (height/2));

				SDL_RenderCopyEx(renderer, currentTexture, NULL, &shipRectangles[index],
								 angleBetweenVectors(&state->playerStates[index].engine, &upVector) + 90,
								 NULL, 0);
			}
		}
		// Present to the screen:
		SDL_RenderPresent(renderer);
		SDL_Delay(1000 / 144);
	}
	pthread_join(networkThread, NULL);
	
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
// compile-command: "gcc `sdl2-config --libs --cflags` Spacewar-Client.c Spacewar-Graphics.c Spacewar-Server.c Spacewar-Physics.c -lSDL2_image -lSDL2_ttf -lm -o 'Spacewar-Client'"
// End:
