// =========================================
// | Spacewar-Messages.h                   |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef SPACEWAR_MESSAGES_H
#define SPACEWAR_MESSAGES_H
#include <stdint.h>

typedef struct SpacewarMessage
{
	uint8_t type;
	uint32_t content;
} SpacewarMessage;

/* Message Types:   
   0 - HELLO: Contents sent to client indicate a given player number.
   1 - GOODBYE: No contents, end the connection.
   2 - PING: Contents indicate the missed amount of pongs.
   3 - PONG: No contents.
   4 - SECRET: Contents indicate the secret key that must be sent with UDP packets to the server.
*/

#endif
// =========================================================
// | End of Spacewar-Messages.h, copyright notice follows. |
// =========================================================

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
