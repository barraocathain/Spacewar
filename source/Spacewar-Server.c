// =========================================
// | Spacewar Server.c                     |
// | Copyright (C) 2023, Barra Ó Catháin   |
// | See end of file for copyright notice. |
// =========================================
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Spacewar-Physics.h"
#include "Spacewar-Messages.h"

void * runServerPhysics(void * parameters)
{
	SpacewarState * state = (SpacewarState *)parameters;

	while (true)
	{
//		doPhysicsTick(state);
		usleep(15625);
	}
}

// Creates a Spacewar server, intended to be ran by the standalone server or forked by the game client:
void * runSpacewarServer(void * configuration)
{
	SpacewarServerConfiguration * serverConfig = (SpacewarServerConfiguration *)configuration;
	printf("Starting Server. \n");
	
	// Initialize a simulation:
	SpacewarState * currentState = calloc(1, sizeof(SpacewarState));
	
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
	sendingAddress.sin_port = htons(serverConfig->port);

	// Create a structure to bind the listening socket:
	struct sockaddr_in listeningAddress;
	memset(&listeningAddress, 0, sizeof(listeningAddress));
	listeningAddress.sin_family = AF_INET; // IPv4
	listeningAddress.sin_addr.s_addr = INADDR_ANY;
	listeningAddress.sin_port = htons(serverConfig->port);

	// Bind to the listening socket:
	if (bind(masterSocket, (const struct sockaddr *)&listeningAddress, sizeof(listeningAddress)) < 0)
    {
		fprintf(stderr, "Failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }
	if (bind(masterListeningSocket, (const struct sockaddr *)&listeningAddress, sizeof(listeningAddress)) < 0)
    {
		fprintf(stderr, "Failed to bind socket.\n");
        exit(EXIT_FAILURE);
    }

	// Begin listening on the master socket:
	listen(masterSocket, 32);
	
	// Create an epoll descriptor to keep track of clients:
	int epollDescriptor = epoll_create(1);

	// Add the master socket to the epoll set:
	struct epoll_event requestedEvents;
	requestedEvents.events = EPOLLIN | EPOLLET;
	requestedEvents.data.fd = masterSocket;	
	epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, masterSocket, &requestedEvents);

	int recievedEventCount = 0;
	struct epoll_event receivedEvents[32];

	// Create a set of connection structs to store the current connection information:
	SpacewarConnection connectedClients[32];
	
	// Begin the simulation:
	pthread_t physicsThread;
	pthread_create(&physicsThread, NULL, runServerPhysics, currentState);
	
	// Manage clients and sending packets back and forth:
	while (true)
	{
		int receivedEventCount = epoll_wait(epollDescriptor, receivedEvents, 32, -1);
		for (int eventIndex = 0; eventIndex < receivedEventCount; eventIndex++)
		{
			// If there's activity on the master socket, there's a new connection:
			if (receivedEvents[eventIndex].data.fd == masterSocket)
			{
				struct sockaddr_in clientAddress;
				socklen_t clientSocketLength = sizeof(struct sockaddr_in);
				int newClientSocket = accept(masterSocket, (struct sockaddr *)&clientAddress,
											 &clientSocketLength);

				// Check that the socket is functional:
				if (newClientSocket < 0)
				{
					fprintf(stderr, "Failed to accept client connection.\n");
					continue;
				}

				// Register the new client in the epoll set:
				requestedEvents.events = EPOLLIN | EPOLLET;
				requestedEvents.data.fd = newClientSocket;
				epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, newClientSocket, &requestedEvents);

				for (int index = 0; index < 32; index++)
				{
					if (connectedClients[index].active == false)
					{
						// Configure the new connection:
						connectedClients[index].active = true;
						connectedClients[index].missedPongs = false;
						connectedClients[index].clientSocket = newClientSocket;
						memcpy(&connectedClients[index].clientAddress, &clientAddress,
							   sizeof(struct sockaddr_in));

						// Send the HELLO packet to the player:
						SpacewarMessage helloMessage;
						helloMessage.type = 0;
						helloMessage.content = index;
						send(newClientSocket, &helloMessage, sizeof(SpacewarMessage), 0);

						// Add the player to the simulation:
						//addPlayer(&connectedClients[index], index, currentState);

						// Send the SECRET packet to the player:
						
						break;
					}
				}
			}
			// Otherwise, we've been sent a packet from one of the connected clients:
			else
			{
				SpacewarMessage receivedMessage;
				size_t bytesRead = recv(receivedEvents->data.fd, &receivedMessage,
										sizeof(SpacewarMessage), 0);

				if (bytesRead == 0)
				{
					// Send a goodbye message:
					SpacewarMessage goodbyeMessage;
					goodbyeMessage.type = 1;
					goodbyeMessage.content = 0;
					send(receivedEvents->data.fd, &goodbyeMessage, sizeof(SpacewarMessage), 0);

					// Remove the socket from the epoll interest set:
					epoll_ctl(epollDescriptor, EPOLL_CTL_DEL, receivedEvents->data.fd, NULL);
					
					// Remove the player from the simulation:
					// removePlayer(&connectedClients[index], currentState);

					// Clear the player struct:
					// clearPlayer(findPlayerBySocket(connectedClients, 32, receivedEvents->data.fd));
					
					// Shutdown the socket:
					shutdown(receivedEvents->data.fd, SHUT_RDWR);
				}

				else
				{
					switch (receivedMessage.content)
					{
						// Handle message contents:
					}
				}
					
			}
		}
	}
	return NULL;
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
