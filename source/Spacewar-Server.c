// =========================================
// | Spacewar Server.c                     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <sys/epoll.h>

void * runServerPhysics(void * parameters)
{
	struct SpacewarState * state = (struct SpacewarState *)parameters;

	while (true)
	{
		doPhysicsTick(state);
		usleep(15625);
	}
}

// Creates a Spacewar server, intended to be ran by the standalone server or forked by the game client:
int runSpacewarServer(uint16_t port)
{
	// Initialize a simulation:
	struct SpacewarState * currentState = calloc(1, sizeof(struct SpacewarState));
	
	// Create our network listeners:
	int masterSocket = socket(AF_INET, SOCK_STREAM, 0);
	int masterListeningSocket = socket(AF_INET, SOCK_DGRAM, 0);
	int masterSendingSocket = socket(AF_INET, SOCK_DGRAM, 0);

	if (masterListeningSocket < 0 || masterSendingSocket < 0 || masterSocket < 0)
	{
		fprintf(stderr, "Failed to create socket.\n");
		exit(EXIT_FAILURE);
	}

	// Make the socket timeout:
	struct timeval readTimeout;
	readTimeout.tv_sec = 0;
	readTimeout.tv_usec = 800;
	setsockopt(masterListeningSocket, SOL_SOCKET, SO_RCVTIMEO, &readTimeout, sizeof(readTimeout));

	// Create a structure to store the address we're sending to:
	struct sockaddr_in sendingAddress;
	sendingAddress.sin_family = AF_INET; // IPv4
	sendingAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	sendingAddress.sin_port = htons(12000);

	// Create a structure to bind the listening socket:
	struct sockaddr_in listeningAddress;
	memset(&listeningAddress, 0, sizeof(listeningAddress));
	listeningAddress.sin_family = AF_INET; // IPv4
	listeningAddress.sin_addr.s_addr = INADDR_ANY;
	listeningAddress.sin_port = port;

	// Bind to the listening socket:
	if (bind(masterSocket, (const struct sockaddr *)&masterListeningAddress, sizeof(listeningAddress)) < 0)
    {
		fprintf(stderr, "Failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }
	if (bind(masterListeningSocket, (const struct sockaddr *)&masterListeningAddress, sizeof(listeningAddress)) < 0)
    {
		fprintf(stderr, "Failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }

	// Begin the simulation:
	pthread_t physicsThread;
	pthread_create(&physicsThread, NULL, runServerPhysics, currentState);

	// Begin listening on the master socket:
	listen(masterSocket, 32);
	
	// Create an epoll descriptor to keep track of clients:
	int epollDescriptor = epoll_create1();

	// Add the master socket to the epoll set:
	struct epoll_event requestedEvents;
	requestedEvents.events = EPOLLIN;
	requestedEvents.data.fd = masterSocket;	
	epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, masterSocket, &requestedEvents);

	int recievedEventCount = 0;
	struct epoll_event receivedEvents[32];
	
	// Manage clients and sending packets back and forth:
	while (true)
	{
		receivedEventCount = epoll_wait(epollDescriptor, receivedEvents, 32, -1);

		for (int eventIndex = 0; eventIndex < receivedEventCount, eventIndex++)
		{
			// If there's activity on the master socket, there's a new connection:
			if (receivedEvents[eventIndex].data.fd == masterSocket)
			{
				struct sockaddr_in clientAddress;
				int newClientSocket = accept(masterSocket, (struct sockaddr *)&clientAddress,
											 sizeof(struct sockaddr_in));

				// Check that the socket is functional:
				if (newClientSocket < 0)
				{
					fprintf(stderr, "Failed to accept client connection.\n");
					continue;
				}

				// Register the new client in the epoll set:
				requestedEvents.events = EPOLLIN;
				requestedEvents.data.fd = newClientSocket;
				epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, newClientSocket, &requestedEvents);


			}
			// Otherwise, we've been sent a packet from one of the connected clients:
			else
			{
				
			}
		}
	}
}

// =======================================================
// | End of Spacewar-Server.c, copyright notice follows. |
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
