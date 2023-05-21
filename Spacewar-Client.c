// Spacewar Client, Barra Ó Catháin.
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
	bool quit = false, rotatingClockwise = false, rotatingAnticlockwise = false, accelerating = false;
	int width = 0, height = 0;
	uint32_t rendererFlags = SDL_RENDERER_ACCELERATED;
	uint64_t thisFrameTime = SDL_GetPerformanceCounter(), lastFrameTime = 0;
	long positionX = 512, positionY = 512, starPositionX = 0, starPositionY = 0;
	xyVector positionVector = {512, 512}, velocityVector = {1, 0}, gravityVector = {0, 0},
		engineVector = {0.04, 0}, upVector = {0, 0.1}, starPosition = {0, 0};

    // Create the socket:
	int receiveSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (receiveSocket < 0)
	{
		fprintf(stderr, "\tSocket Creation is:\t\033[33;40mRED.\033[0m Aborting launch.\n");
		exit(0);
	}
	printf("\tSocket Creation is:\t\033[32;40mGREEN.\033[0m\n");

	// Make the socket timeout:
	struct timeval read_timeout;
	read_timeout.tv_sec = 0;
	read_timeout.tv_usec = 8;
	setsockopt(receiveSocket, SOL_SOCKET, SO_RCVTIMEO, &read_timeout, sizeof(read_timeout));
	
	// Create and fill the information needed to bind to the socket:
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET; // IPv4
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(12000);

	// Bind to the socket:
	if (bind(receiveSocket, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

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
	memset(&sendAddress, 0, sizeof(sendAddress));
	sendAddress.sin_family = AF_INET; // IPv4
	sendAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	sendAddress.sin_port = htons(12001);
	
	// Get the initial
	ship shipA;
	ship shipB;
	
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
	SDL_Window * window = SDL_CreateWindow("SDL_TEST", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 450, 0);
	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, rendererFlags);
	SDL_SetWindowTitle(window, "Spacewar!");
	
	// Load in all of our textures:
	SDL_Texture * idleTexture, * acceleratingTexture, * clockwiseTexture, * anticlockwiseTexture, * currentTexture,
	    * acceleratingTexture2, * blackHoleTexture;
	
	idleTexture = IMG_LoadTexture(renderer, "./Images/Ship-Idle.png");
	clockwiseTexture = IMG_LoadTexture(renderer, "./Images/Ship-Clockwise.png");
	acceleratingTexture = IMG_LoadTexture(renderer, "./Images/Ship-Accelerating.png");
	anticlockwiseTexture = IMG_LoadTexture(renderer, "./Images/Ship-Anticlockwise.png");
	acceleratingTexture2 = IMG_LoadTexture(renderer, "./Images/Ship-Accelerating-Frame-2.png");

	// Load the starfield texture:
	SDL_Texture * starfieldTexture = IMG_LoadTexture(renderer, "./Images/Starfield.png");
	SDL_Rect starfieldRect;
	SDL_QueryTexture(starfieldTexture, NULL, NULL, NULL, &starfieldRect.h);
	SDL_QueryTexture(starfieldTexture, NULL, NULL, &starfieldRect.w, NULL);

	blackHoleTexture = IMG_LoadTexture(renderer, "./Images/Black-Hole.png");
	SDL_Rect blackHoleRectangle;
	blackHoleRectangle.x = 0;
	blackHoleRectangle.y = 0;
	SDL_QueryTexture(blackHoleTexture, NULL, NULL, NULL, &blackHoleRectangle.h);
	SDL_QueryTexture(blackHoleTexture, NULL, NULL, &blackHoleRectangle.w, NULL);


	// Enable resizing the window:
	SDL_SetWindowResizable(window, SDL_TRUE);
	ship Temp;
	playerController playerOne;
	playerOne.number = 1;
	bool shipAUpdated, shipBUpdated;

	TTF_Font * font = TTF_OpenFont("./Robtronika.ttf", 12);
	SDL_Color white = {255, 255, 255};
	int keyCount = 0; 
	const uint8_t * keyboardState = SDL_GetKeyboardState(&keyCount);

	// Prep the titlescreen struct:
	SpacewarTitlescreen titlescreen = prepareTitleScreen(window, renderer, "./Images/Starfield.png",
														 "./Images/Title.png", font,
														 "Press Enter or Button 0 on your joystick to connect!");
		
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

	while (!quit)
	{
		while(!(shipAUpdated && shipBUpdated))
		{
			// Receive data from the socket:
			recvfrom(receiveSocket, &Temp, sizeof(ship), 0, NULL, NULL);
			if(Temp.number == 0)
			{
				shipA = Temp;
				shipAUpdated = true;
			}
			if(Temp.number == 1)
			{
				shipB = Temp;
				shipBUpdated = true;
			}
		}
	
		// Check if the user wants to quit:
		while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
			case SDL_QUIT:
			{
				quit = true;
				break;
			}
			case SDL_KEYDOWN:
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_LEFT:
				{
					playerOne.turningAnticlockwise = true;
					break;
				}
				case SDLK_RIGHT:
				{
					playerOne.turningClockwise = true;
					break;
				}
				case SDLK_UP:
				{
					playerOne.accelerating = true;
					break;
				}
				default:
				{
					break;
				}
				}
				break;
			}
			case SDL_KEYUP:
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_LEFT:
				{
					playerOne.turningAnticlockwise = false;
					break;
				}
				case SDLK_RIGHT:
				{
					playerOne.turningClockwise = false;
					break;
				}
				case SDLK_UP:
				{
					playerOne.accelerating = false;
					break;
				}
				}
			}
			default:
			{
				break;
			}	   
			break;
			}
		}
		sendto(sendSocket, &playerOne, sizeof(playerOne), 0, (const struct sockaddr *)&sendAddress, sizeof(sendAddress));
        // Store the window's current width and height:
		SDL_GetWindowSize(window, &width, &height);

		// Set the texture to idle:
		currentTexture = idleTexture;
		
		// Calculate the position of the sprites:
		shipB.rectangle.x = (width/2) - 16 - (shipB.velocity.xComponent * 15);
		shipB.rectangle.y = (height/2) - 16 - (shipB.velocity.yComponent * 15);

		shipA.rectangle.x = (long)((((shipA.position.xComponent - shipB.position.xComponent) - 32) + width/2) - (shipB.velocity.xComponent * 15));
		shipA.rectangle.y = (long)((((shipA.position.yComponent - shipB.position.yComponent) - 32) + height/2) - (shipB.velocity.yComponent * 15));
		
		// Set the colour to black:
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		// Clear the screen, filling it with black:
		SDL_RenderClear(renderer);

		// Draw the starfield:
		starfieldRect.x = -900 - (long)shipB.position.xComponent % 800 - (shipB.velocity.xComponent * 15);
		starfieldRect.y = -900 - (long)shipB.position.yComponent % 800 - (shipB.velocity.yComponent * 15);
		while(starfieldRect.x <= (width + 800))
		{
			while(starfieldRect.y <= (height + 800))
			{
				SDL_RenderCopy(renderer, starfieldTexture,  NULL,  &starfieldRect);
				starfieldRect.y += 800;
			}
			starfieldRect.y = -900 - (long)shipB.position.yComponent % 800  - (shipB.velocity.yComponent * 15);
			starfieldRect.x += 800;
		}
		
		// Draw the ship:
		SDL_RenderCopyEx(renderer, currentTexture, NULL, &shipA.rectangle,
						 angleBetweenVectors(&shipA.engine, &upVector) + 90, NULL, 0);
		SDL_RenderCopyEx(renderer, currentTexture, NULL, &shipB.rectangle,
						 angleBetweenVectors(&shipB.engine, &upVector) + 90, NULL, 0);
		
		// Calculate the position of the black hole on screen and render it:
		blackHoleRectangle.x = ((long)(starPositionX - shipB.position.xComponent - (blackHoleRectangle.w / 2)) + width/2)
			- (shipB.velocity.xComponent * 15);
		blackHoleRectangle.y = ((long)(starPositionY - shipB.position.yComponent  - (blackHoleRectangle.h / 2)) + height/2)
			- (shipB.velocity.yComponent * 15);
		SDL_RenderCopy(renderer, blackHoleTexture, NULL, &blackHoleRectangle);

		// Present the rendered graphics:
		SDL_RenderPresent(renderer);
		shipAUpdated = false;
		shipBUpdated = false;
	}
	return 0;
}
// ========================================================================================================
// Local Variables:
// compile-command: "gcc `sdl2-config --libs --cflags` Spacewar-Client.c spacewarPlayer.c spacewarGraphics.c -lSDL2_image -lSDL2_ttf -lm -o 'Spacewar Client!'"
// End:
