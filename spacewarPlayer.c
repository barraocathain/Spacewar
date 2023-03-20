// spacewarPlayer.c: Contains function definitions for player interaction.
// Barra Ó Catháin, 2023
// =======================================================================
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "xyVector.h"
#include "spacewarPlayer.h"

void takeNetworkInput(playerController * controller, int descriptor)
{
	recvfrom(descriptor, controller, sizeof(playerController), 0, NULL, NULL);
}

void getPlayerInput(playerController * controller)
{
	SDL_PumpEvents();
	const uint8_t * keyboardState = SDL_GetKeyboardState(NULL);
	if(keyboardState[SDL_SCANCODE_UP] == 1)
	{
		controller->accelerating = true;
	}
	else
	{
		controller->accelerating = false;
	}
	if(keyboardState[SDL_SCANCODE_LEFT] == 1)
	{
		controller->turningAnticlockwise = true;
	}
	else
	{
		controller->turningAnticlockwise = false;;
	}
	if(keyboardState[SDL_SCANCODE_RIGHT] == 1)
	{
		controller->turningClockwise = true;
	}
	else
	{
		controller->turningClockwise = false;
	}
	if(controller->joystick != NULL)
	{
		controller->turningAmount = SDL_JoystickGetAxis(controller->joystick, 0);
		controller->acceleratingAmount = SDL_JoystickGetAxis(controller->joystick, 5);
	}
}

void doShipInput(playerController * controller, ship * ship, xyVector starPosition, double deltaTime)
{
	if(controller->number == ship->number)
	{
		// Calculate the gravity for the ships:
		calculateGravity(&starPosition, ship);
				
		// Rotate the engine vector if needed:
		if (controller->turningClockwise)
		{
			rotateXYVector(&ship->engine, 0.25 * deltaTime);
		}
		else if (controller->turningAmount > 2500)
		{
			double rotationalSpeed = (controller->turningAmount / 20000);
			rotateXYVector(&ship->engine, 0.25 * deltaTime * rotationalSpeed);
		}
		
		if (controller->turningAnticlockwise)
		{
			rotateXYVector(&ship->engine, -0.25 * deltaTime);	
		}
		else if (controller->turningAmount < -2500)
		{
			double rotationalSpeed = (controller->turningAmount / 20000);
			rotateXYVector(&ship->engine, 0.25 * deltaTime * rotationalSpeed);
		}
		
		// Calculate the new current velocity:
		addXYVectorDeltaScaled(&ship->velocity, &ship->gravity, deltaTime);

		if (controller->acceleratingAmount > 2500)
		{
			xyVector temporary = ship->engine;
			multiplyXYVector(&ship->engine, controller->acceleratingAmount/ 32748);
			SDL_HapticRumblePlay(controller->haptic, (float)controller->acceleratingAmount / 32768, 20);
			addXYVectorDeltaScaled(&ship->velocity, &ship->engine, deltaTime);
			ship->engine = temporary;
		}
		
		else if (controller->accelerating)
		{
			addXYVectorDeltaScaled(&ship->velocity, &ship->engine, deltaTime);
		}
		
		// Calculate the new position:
		addXYVectorDeltaScaled(&ship->position, &ship->velocity, deltaTime);
	}
}
