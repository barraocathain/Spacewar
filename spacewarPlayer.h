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

static inline playerController createShipPlayerController(ship * ship)
{
	playerController newController;
	newController.number = ship->number;
	newController.joystick = NULL;
	return newController;
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

// Function prototypes:
void doShipInput(playerController * controller, ship * ship, xyVector starPosition, double deltaTime);

void takeNetworkInput(playerController * controller, int descriptor);

void getPlayerInput(playerController * controller);
	
#endif
