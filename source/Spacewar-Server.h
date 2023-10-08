// =========================================
// | Spacewar-Server.h                     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef SPACEWAR_SERVER
#define SPACEWAR_SERVER

typedef struct SpacewarConnection
{
	uint8_t missedPongs;
	int clientSocket;
	int playerNumber;
	struct sockaddr_in clientAddress;
} SpacewarConnection;

// Creates a spacewar server, intended to be ran by the standalone server or forked by the game client:
int runSpacewarServer(uint16_t port);

#endif
// =======================================================
// | End of Spacewar-Server.h, copyright notice follows. |
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
