// Spacewar, Barra Ó Catháin.
// ===================================
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "xyVector.h"
#include "spacewarPlayer.h"
#include "spacewarGraphics.h"

int main(int argc, char ** argv)
{
	SDL_Event event;
	int width = 0, height = 0;
	uint32_t rendererFlags = SDL_RENDERER_ACCELERATED;
	uint64_t thisFrameTime = SDL_GetPerformanceCounter(), lastFrameTime = 0;
	long starPositionX = 0, starPositionY = 0;
	double deltaTime = 0, frameAccumulator = 0;	
	bool quit = false, rotatingClockwise = false, rotatingAnticlockwise = false, accelerating = false;
	xyVector engineVector = {0.85, 0}, upVector = {0, 0.1}, starPosition = {0, 0};

	// Create the socket:
	int sendSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (sendSocket < 0)
	{
		fprintf(stderr, "\tSocket Creation is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	printf("\tSocket Creation is:\t\033[32;40mGREEN.\033[0m\n");

	// Create and fill the information needed to bind to the socket:
	struct sockaddr_in sendAddress;
	sendAddress.sin_family = AF_INET; // IPv4
	sendAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	sendAddress.sin_port = htons(12000);

	int receiveSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (receiveSocket < 0)
	{
		fprintf(stderr, "\tSocket Creation is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	printf("\tSocket Creation is:\t\033[32;40mGREEN.\033[0m\n");

	// Make the socket timeout:
	struct timeval readTimeout;
	readTimeout.tv_sec = 0;
	readTimeout.tv_usec = 800;
	setsockopt(receiveSocket, SOL_SOCKET, SO_RCVTIMEO, &readTimeout, sizeof(readTimeout));
	
	// Create and fill the information needed to bind to the socket:
	struct sockaddr_in receiveAddress;
	memset(&receiveAddress, 0, sizeof(receiveAddress));
	receiveAddress.sin_family = AF_INET; // IPv4
	receiveAddress.sin_addr.s_addr = INADDR_ANY;
	receiveAddress.sin_port = htons(12001);

	// Bind to the socket:
	if (bind(receiveSocket, (const struct sockaddr *)&receiveAddress, sizeof(receiveAddress)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

	ship shipA = createShip(32, 32, 512, 512, 1, 0, 0);
	ship shipB = createShip(32, 32, -512, -512, 0, 1, 1);
	
	// Initialize the SDL library, video, sound, and input:
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL Initialization Error: %s\n", SDL_GetError());
	}
	
	// Initialize image loading:
	IMG_Init(IMG_INIT_PNG);

	// Initialize font support:
	TTF_Init();
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");
	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	
	playerController playerOne = createShipPlayerController(&shipA);
	playerController playerTwo = createShipPlayerController(&shipB);

	// Create an SDL window and rendering context in that window:
	SDL_Window * window = SDL_CreateWindow("SDL_TEST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 700, 700, 0);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, rendererFlags);

	// Set some properties for the window:
	SDL_SetWindowResizable(window, SDL_TRUE);
	SDL_SetWindowTitle(window, "Spacewar!");

	int keyCount = 0; 
	const uint8_t * keyboardState = SDL_GetKeyboardState(&keyCount);

	SDL_Texture * starfieldTexture = IMG_LoadTexture(renderer, "./Images/Starfield.png");
	SDL_Rect starfieldRect;
	SDL_QueryTexture(starfieldTexture, NULL, NULL, NULL, &starfieldRect.h);
	SDL_QueryTexture(starfieldTexture, NULL, NULL, &starfieldRect.w, NULL);

	TTF_Font * font = TTF_OpenFont("./Robtronika.ttf", 12);
	SDL_Color white = {255, 255, 255};
	
	// Check for joysticks:
	if (SDL_NumJoysticks() < 1 )
	{
		playerOne.joystick = NULL;
		bool inputSelected = false;
		
		// Prep the titlescreen struct:
		SpacewarTitlescreen titlescreen = prepareTitleScreen(window, renderer, "./Images/Starfield.png",
															 "./Images/Title.png", font, "Press Enter to play.");

		// Render the title text:
		while(!inputSelected)
		{
			// Draw the title screen according to the titlescreen struct:
			drawTitleScreen(&titlescreen);

			// Wait until they press enter:
			SDL_PumpEvents();
			SDL_GetKeyboardState(&keyCount);
			if(keyboardState[SDL_SCANCODE_RETURN] == 1)
			{		   
				inputSelected = true;
			}

			// Delay enough so that we run at 30 frames in the menu:
			SDL_Delay(1000 / 30);
		}	
	}
	else
	{
		// Prep the titlescreen struct:
		SpacewarTitlescreen titlescreen = prepareTitleScreen(window, renderer, "./Images/Starfield.png",
															 "./Images/Title.png", font,
															 "Press Enter or Button 0 on your joystick to play.");
		
		// Load all joysticks:
		int joystickListLength = SDL_NumJoysticks();
		SDL_Joystick ** joysticksList = calloc(joystickListLength, sizeof(SDL_Joystick*));
		
		for(int index = 0; index < SDL_NumJoysticks(); index++)
		{
			joysticksList[index] = SDL_JoystickOpen(index);
		}

		// Choose a player joystick:
		printf("Please press button zero on the controller you wish to use, or enter to play keyboard only.\n");
		
		int joystickIndex = 0;
		bool inputSelected = false;

		// Render the title text:
		while(!inputSelected)
		{
			// Draw the title screen according to the titlescreen struct:
			drawTitleScreen(&titlescreen);

			SDL_PumpEvents();
			SDL_GetKeyboardState(&keyCount);
			if(keyboardState[SDL_SCANCODE_RETURN] == 1)
			{
				joysticksList[joystickIndex] = NULL;
				inputSelected = true;
			}
			joystickIndex++;
			if(joystickIndex >= joystickListLength)
			{
				joystickIndex = 0;
			}
			if(SDL_JoystickGetButton(joysticksList[joystickIndex], 0) == 1)
			{
				inputSelected = true;				
			}
			
			// Delay enough so that we run at 30 frames in the menu:
			SDL_Delay(1000 / 30);
		}
		
		// Load joystick
		playerOne.joystick = joysticksList[joystickIndex];
		if (playerOne.joystick == NULL )
		{
			printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
		}
		playerOne.haptic = SDL_HapticOpenFromJoystick(playerOne.joystick);
		SDL_HapticRumbleInit(playerOne.haptic);
	}


	// Load in all of our textures:
	SDL_Texture * blackHoleTexture, * idleTexture, * acceleratingTexture, * clockwiseTexture, * anticlockwiseTexture,
		* currentTexture, * acceleratingTexture2, *blackHolePointerTexture;

	idleTexture = IMG_LoadTexture(renderer, "./Images/Ship-Idle.png");
	blackHoleTexture = IMG_LoadTexture(renderer, "./Images/Black-Hole.png");
	clockwiseTexture = IMG_LoadTexture(renderer, "./Images/Ship-Clockwise.png");
	acceleratingTexture = IMG_LoadTexture(renderer, "./Images/Ship-Accelerating.png");
	anticlockwiseTexture = IMG_LoadTexture(renderer, "./Images/Ship-Anticlockwise.png");
	blackHolePointerTexture = IMG_LoadTexture(renderer, "./Images/Black-Hole-Pointer.png");
	acceleratingTexture2 = IMG_LoadTexture(renderer, "./Images/Ship-Accelerating-Frame-2.png");
	currentTexture = acceleratingTexture;

	SDL_Rect blackHoleRectangle;
	blackHoleRectangle.x = 0;
	blackHoleRectangle.y = 0;
	SDL_QueryTexture(blackHoleTexture, NULL, NULL, NULL, &blackHoleRectangle.h);
	SDL_QueryTexture(blackHoleTexture, NULL, NULL, &blackHoleRectangle.w, NULL);

	SDL_Rect blackHolePointerRectangle;
	blackHolePointerRectangle.x = 0;
	blackHolePointerRectangle.y = 0;
	blackHolePointerRectangle.w = 128;
	blackHolePointerRectangle.h = 128;
	lastFrameTime = SDL_GetPerformanceCounter();
	thisFrameTime = SDL_GetPerformanceCounter();
	
	while (!quit)
	{
		lastFrameTime = thisFrameTime;
		thisFrameTime = SDL_GetPerformanceCounter();
		deltaTime = (double)(((thisFrameTime - lastFrameTime) * 1000) / (double)SDL_GetPerformanceFrequency());

		sendto(sendSocket, &shipA, sizeof(ship), 0, (const struct sockaddr *)&sendAddress, sizeof(sendAddress));
		sendto(sendSocket, &shipB, sizeof(ship), 0, (const struct sockaddr *)&sendAddress, sizeof(sendAddress));
		// Store the window's current width and height:
		SDL_GetWindowSize(window, &width, &height);
		
		// Check input:
		while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
				case SDL_QUIT:
				{
					quit = true;
					break;
				}
			}
		}
		
		// Wrap the positions if the ship goes interstellar:
		if(shipA.position.xComponent > 4096)
		{
			shipA.position.xComponent = -2000;
		}
		else if(shipA.position.xComponent < -4096)
		{
			shipA.position.xComponent = 2000;
		}
		if(shipA.position.yComponent > 4096)
		{
			shipA.position.yComponent = -2000;
		}
		else if(shipA.position.yComponent < -4096)
		{
			shipA.position.yComponent = 2000;
		}

		if(shipB.position.xComponent > 4096)
		{
			shipB.position.xComponent = -2000;
			shipB.velocity.xComponent *= 0.9;
		}
		else if(shipB.position.xComponent < -4096)
		{
			shipB.position.xComponent = 2000;
			shipB.velocity.xComponent *= 0.9;
		}
		if(shipB.position.yComponent > 4096)
		{
			shipB.position.yComponent = -2000;
			shipB.velocity.yComponent *= 0.9;
		}
		else if(shipB.position.yComponent < -4096)
		{
			shipB.position.yComponent = 2000;
			shipB.velocity.yComponent *= 0.9;
		}

		// Get the needed input:
		getPlayerInput(&playerOne);
		takeNetworkInput(&playerTwo, receiveSocket);

		// Do the needed input:
		doShipInput(&playerOne, &shipA, starPosition, deltaTime);
		doShipInput(&playerTwo, &shipB, starPosition, deltaTime);
		
		shipA.rectangle.x = (width/2) - 16 - (shipA.velocity.xComponent * 15);
		shipA.rectangle.y = (height/2) - 16 - (shipA.velocity.yComponent * 15);

		shipB.rectangle.x = (long)((((shipB.position.xComponent - shipA.position.xComponent) - 32) + width/2) - (shipA.velocity.xComponent * 15));
		shipB.rectangle.y = (long)((((shipB.position.yComponent - shipA.position.yComponent) - 32) + height/2) - (shipA.velocity.yComponent * 15));
		

		// Set the colour to black:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		// Clear the screen, filling it with black:
		SDL_RenderClear(renderer);

		starfieldRect.x = -900 - (long)shipA.position.xComponent % 800 - (shipA.velocity.xComponent * 15);
		starfieldRect.y = -900 - (long)shipA.position.yComponent % 800 - (shipA.velocity.yComponent * 15);
		while(starfieldRect.x <= (width + 800))
		{
			while(starfieldRect.y <= (height + 800))
			{
				SDL_RenderCopy(renderer, starfieldTexture,  NULL,  &starfieldRect);
				starfieldRect.y += 800;
			}
			starfieldRect.y = -900 - (long)shipA.position.yComponent % 800  - (shipA.velocity.yComponent * 15);
			starfieldRect.x += 800;
		}
			
		// Draw the ship:
		SDL_RenderCopyEx(renderer, currentTexture, NULL, &shipA.rectangle,
						 angleBetweenVectors(&shipA.engine, &upVector) + 90, NULL, 0);
		SDL_RenderCopyEx(renderer, currentTexture, NULL, &shipB.rectangle,
						 angleBetweenVectors(&shipB.engine, &upVector) + 90, NULL, 0);

		// Draw the black hole:
		blackHoleRectangle.x = ((long)(starPositionX - shipA.position.xComponent - (blackHoleRectangle.w / 2)) + width/2)
			- (shipA.velocity.xComponent * 15);
		blackHoleRectangle.y = ((long)(starPositionY - shipA.position.yComponent  - (blackHoleRectangle.h / 2)) + height/2)
			- (shipA.velocity.yComponent * 15);
		SDL_RenderCopy(renderer, blackHoleTexture, NULL, &blackHoleRectangle);

		// Set the colour to yellow:
		SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
		
		// Draw a line representing the velocity:
		SDL_RenderDrawLine(renderer, width/2 - (shipA.velocity.xComponent * 15),
						   height/2  - (shipA.velocity.yComponent * 15),
						   (long)((width/2) + shipA.velocity.xComponent * 15)  - (shipA.velocity.xComponent * 15),
	   					   (long)((height/2) + shipA.velocity.yComponent * 15)  - (shipA.velocity.yComponent * 15));

		// Set the colour to blue:
		SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);

		// Draw a line representing the direction of the star:
		blackHolePointerRectangle.x = width/2  - (shipA.velocity.xComponent * 15) - (blackHolePointerRectangle.w / 2);
		blackHolePointerRectangle.y = height/2  - (shipA.velocity.yComponent * 15) -  (blackHolePointerRectangle.h / 2);
		SDL_RenderCopyEx(renderer, blackHolePointerTexture, NULL, &blackHolePointerRectangle,
						 angleBetweenVectors(&shipA.gravity, &upVector) + 90, NULL, 0);
		
		// Present the rendered graphics:
		SDL_RenderPresent(renderer);
	}
	return 0;
}
// ========================================================================================================
// Local Variables:
// compile-command: "gcc `sdl2-config --libs --cflags` Spacewar.c spacewarPlayer.c spacewarGraphics.c -lSDL2_image -lSDL2_ttf -lm -o 'Spacewar!'"
// End:
