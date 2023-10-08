// =========================================
// | Spacewar-Physics.h                    |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef SPACEWAR_PHYSICS
#define SPACEWAR_PHYSICS

#include <stdint.h>
#include "xyVector.h"

typedef struct SpacewarShipState
{
	bool inPlay;
	xyVector engine;
	xyVector gravity;
	xyVector position;
	xyVector velocity;	
} SpacewarShipState;

typedef struct SpacewarClientInput
{
	double turningAmount, acceleratingAmount;
	bool turningClockwise, turningAnticlockwise, accelerating;
} SpacewarClientInput;

typedef struct SpacewarState
{
	uint64_t tickNumber;
	struct timeval timestamp;
	SpacewarShipState playerStates[32];
	SpacewarClientInput playerInputs[32];
} SpacewarState;

// Does a single step of the physics:
void doPhysicsTick(SpacewarState * state);

#endif
// ========================================================
// | End of Spacewar-Physics.h, copyright notice follows. |
// ========================================================

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
