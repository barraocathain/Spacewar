// =========================================
// | Spacewar-Server.h                     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#ifndef SPACEWAR_SERVER
#define SPACEWAR_SERVER
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Spacewar-Physics.h"

typedef struct SpacewarState SpacewarState;

typedef struct SpacewarConnection
{
	bool active;
	int clientSocket;
	uint8_t missedPongs;
	uint32_t playerSecret;
	struct sockaddr_in clientAddress;
} SpacewarConnection;


typedef struct SpacewarServerConfiguration
{
	uint16_t port;
} SpacewarServerConfiguration;


typedef struct SpacewarServerSharedState
{
	SpacewarState * state;
	SpacewarConnection * connections;
} SpacewarServerSharedState;

// Creates a spacewar server, intended to be ran by the standalone server or forked by the game client:
void * runSpacewarServer(void * configuration);

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
// along with this program. If not, see <https://www.gnu.org/licenses/>.
