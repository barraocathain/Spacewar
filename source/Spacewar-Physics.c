// =========================================
// | Spacewar-Physics.c                    |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include "Spacewar-Physics.h"

void doPhysicsTick(SpacewarState * state)
{
	double gravityMagnitude, gravityAcceleration;
	for (int shipIndex = 0; shipIndex < 32; shipIndex++)
	{
		SpacewarShipState * currentShip = state->playerStates[shipIndex];
		if (currentShip->inPlay)
		{
			// Calculate Gravity:
			xyVectorBetweenPoints(currentShip->position.xComponent, currentShip->position.yComponent,
								  0, 0, currentShip.gravity);
			gravityMagnitude = normalizeXYVector(currentShip.gravity);
			gravityAcceleration = 0;

			// Some maths that felt okay:
			if (gravityMagnitude >= 116)
			{
				gravityAcceleration = pow(2, (3000 / (gravityMagnitude << 1)))) >> 3;
			}
			// We're actually in the black hole; teleport:
			else
			{
				ship->position.xComponent = random() % 8000;
				ship->position.yComponent = random() % 8000;
				ship->velocity.xComponent *= 0.01;
				ship->velocity.yComponent *= 0.01;
			}
		
		 	multiplyXYVector(currentShip.gravity, gravityAcceleration);
			
			// Apply Inputs:
			
			// Apply Gravity and Velocity to Position:
			addXYVector(currentShip.position, currentShip.gravity);
			
			// Wrap position to game field: 
			if (currentShip->position.xComponent > 8000)
			{
				state->playerStates[shipIndex].position.xComponent = -7999;
				state->playerStates[shipIndex].velocity.xComponent *= 0.9;
			}
			if (currentShip->position.xComponent < -8000)
			{
				state->playerStates[shipIndex].position.xComponent = 7999;
				state->playerStates[shipIndex].velocity.xComponent *= 0.9;
			}
			if (currentShip->position.yComponent > 8000)
			{
				state->playerStates[shipIndex].position.yComponent = -7999;
				state->playerStates[shipIndex].velocity.yComponent *= 0.9;
			}
			if (currentShip->position.yComponent < -8000)
			{
				state->playerStates[shipIndex].position.xComponent = 7999;
				state->playerStates[shipIndex].velocity.yComponent *= 0.9;
			}			
		}
	}
}
// ========================================================
// | End of Spacewar-Physics.c, copyright notice follows. |
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
