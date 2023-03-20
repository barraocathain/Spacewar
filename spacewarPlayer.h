#ifndef SPACEWARPLAYER_H
#define SPACEWARPLAYER_H
#include "xyVector.h"

// A struct storing the needed data to draw a ship:
typedef struct ship
{
	int number;
	xyVector engine;
	xyVector gravity;
	xyVector position;
	xyVector velocity;
	SDL_Rect rectangle;
} ship;

// A struct to store the input state for one player:
typedef struct playerController
{
	SDL_Joystick * joystick;
	SDL_Haptic * haptic;
	int number;
	double turningAmount, acceleratingAmount;
	bool turningClockwise, turningAnticlockwise, accelerating;	
} playerController;

static inline void calculateGravity(xyVector * starPosition, ship * shipUnderGravity)
{
	// Calculate the vector between the star and ship:
	xyVectorBetweenPoints(shipUnderGravity->position.xComponent, shipUnderGravity->position.yComponent,
						  starPosition->xComponent, starPosition->yComponent, &shipUnderGravity->gravity);
		
	// Make it into a unit vector:
	double gravityMagnitude = normalizeXYVector(&shipUnderGravity->gravity);
	double gravityAcceleration = 0;
	
	// Calculate the gravity between the star and ship:
	if(gravityMagnitude != 0)
	{
		if(gravityMagnitude >= 116)
		{
			gravityAcceleration = pow(2, (3000 / (pow(gravityMagnitude, 2)))) / 8;
		}
		else
		{
			gravityAcceleration = 1;
		}
	}
	else
	{
		gravityAcceleration = 1;
	}
	
	if(gravityAcceleration < 0.01)
	{
		gravityAcceleration = 0.01;
	}
		
	// Scale the vector:
	multiplyXYVector(&shipUnderGravity->gravity, gravityAcceleration);	
}

static inline void takeNetworkInput(playerController * controller, int descriptor)
{
	recvfrom(descriptor, controller, sizeof(playerController), 0, NULL, NULL);
}


static inline void getPlayerInput(playerController * controller, int playerNumber)
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

static inline playerController createShipPlayerController(ship * ship)
{
	playerController newController;
	newController.number = ship->number;
	return newController;
}


static inline void doShipInput(playerController * controller, ship * ship, xyVector starPosition, double deltaTime)
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

// Create a ship with the given parameters:
static inline ship createShip(int width, int height, double positionX, double positionY, double velocityX, double velocityY, int number)
{
	ship newShip;

	// Player number:
	newShip.number = number;
	
	// Rectangle to show the ship in:
	newShip.rectangle.w = width;
	newShip.rectangle.h = height;
	
	// Position:
	newShip.position.xComponent = positionX;
	newShip.position.yComponent = positionY;

	// Velocity:
	newShip.velocity.xComponent = velocityX;
	newShip.velocity.yComponent = velocityY;

	// Gravity:
	newShip.gravity.xComponent = 0;
	newShip.gravity.yComponent = 0;

	// Engine:
	newShip.engine.yComponent = 0;
	newShip.engine.xComponent = 0.1;   	
	return newShip;
}


#endif
