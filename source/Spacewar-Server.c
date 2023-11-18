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
#include "Spacewar-Messages.h"
#include "Spacewar-Physics.h"

SpacewarConnection * getConnectionBySocket(SpacewarConnection * connections, size_t connectionCount, int socket)
{
	for (size_t connectionIndex = 0; connectionIndex < connectionCount; connectionIndex++)
	{
		if (connections[connectionIndex].clientSocket == socket)
		{
			return &connections[connectionIndex];
		}
	}
	return NULL;
}

void sendCurrentState(SpacewarState * state, SpacewarConnection * connections, int udpSocket)
{
	for (int connectionIndex = 0; connectionIndex < 32; connectionIndex++)
	{
		if (connections[connectionIndex].active)
		{
			sendto(udpSocket, state, sizeof(SpacewarState), 0,
				   (struct sockaddr *)&connections[connectionIndex].clientAddress,
				   sizeof(struct sockaddr_in));
		}
	}
}
void * runServerPhysics(void * parameters)
{
	SpacewarServerSharedState * sharedState = (SpacewarServerSharedState *)parameters;
	for(int index = 0; index < 32; index++)
	{
		sharedState->physicsState->playerStates[index].engine.yComponent = 0.1;
	}
	
	while (true)
	{
		doPhysicsTick(sharedState->physicsState);
		sendCurrentState(sharedState->physicsState, sharedState->connections, sharedState->udpSocket);		
		usleep(15625);
	}

	return NULL;
}

void * runInputReceiver(void * parameters)
{
	SpacewarServerSharedState * sharedState = (SpacewarServerSharedState *)parameters;
	int bytesRead;
	socklen_t socketAddressLength;
	struct sockaddr_in clientAddress;

	SpacewarClientInput input;

	while (true)
	{
		bytesRead = recvfrom(sharedState->udpSocket, &input, sizeof(SpacewarClientInput), 0,
							 (struct sockaddr *)&clientAddress, &socketAddressLength);
		if (bytesRead == sizeof(SpacewarClientInput))
		{
			if (input.playerNumber < 32)
			{
				if (input.secret == sharedState->connections[input.playerNumber].playerSecret)
				{
					sharedState->physicsState->playerStates[input.playerNumber].inPlay = true;
					memcpy(&sharedState->connections[input.playerNumber].clientAddress,
						   &clientAddress, sizeof(struct sockaddr_in));
					memcpy(&sharedState->physicsState->playerInputs[input.playerNumber], &input.input,
						   sizeof(SpacewarShipInput));
				}
			}
		}
		bzero(&input, sizeof(SpacewarClientInput));
	}

	return NULL;
}

// Adds a new player to a physics simulation. Returns a randomly generated secret key:
uint32_t addPlayer(SpacewarConnection * connection, int playerNumber, SpacewarState * state)
{
	connection->playerSecret = rand();
	state->playerStates[playerNumber].inPlay = false;
	
	return connection->playerSecret;
}

// Creates a Spacewar server, intended to be ran by the standalone server or forked by the game client:
void * runSpacewarServer(void * configuration)
{
	SpacewarServerConfiguration * serverConfig = (SpacewarServerConfiguration *)configuration;
	printf("Starting Server.\n");
		
	// Create our network listeners:
	int masterSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (masterSocket < 0)
	{
		fprintf(stderr, "Failed to create socket.\n");
		exit(EXIT_FAILURE);
	}
	setsockopt(masterSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	setsockopt(masterSocket, SOL_SOCKET, SO_REUSEPORT, &(int){1}, sizeof(int));

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

	// Begin listening on the master socket:
	listen(masterSocket, 32);
	
	// Create an epoll descriptor to keep track of clients:
	int recievedEventCount = 0;
	struct epoll_event receivedEvents[32];
	int epollDescriptor = epoll_create(1);

	// Add the master socket to the epoll set:
	struct epoll_event requestedEvents;
	requestedEvents.events = EPOLLIN | EPOLLET;
	requestedEvents.data.fd = masterSocket;	
	epoll_ctl(epollDescriptor, EPOLL_CTL_ADD, masterSocket, &requestedEvents);

	// Create a set of connection structs to store the current connection information:
	SpacewarConnection * connectedClients = calloc(32, sizeof(SpacewarConnection));
	for(int index = 0; index < 32; index++)
	{
		connectedClients[index].active = false;
	}

	// Create a UDP socket:
	int udpSocket = 0;
	if ((udpSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		exit(EXIT_FAILURE);
	}

	// Create a struct to bind the UDP socket to a port:
	struct sockaddr_in serverAddress;
	memset(&serverAddress, 0, sizeof(serverAddress));       
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(5200);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

	// Bind it:
	bind(udpSocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in));
	
	// Setup the server threads:
	pthread_t physicsThread, inputThread;
	SpacewarState * currentState = calloc(1, sizeof(SpacewarState));
	SpacewarServerSharedState threadState;
	threadState.udpSocket = udpSocket;
	threadState.physicsState = currentState;
	threadState.connections = connectedClients;

	// Begin the simulation:
	pthread_create(&inputThread, NULL, runInputReceiver, &threadState);
	pthread_create(&physicsThread, NULL, runServerPhysics, &threadState);
	
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
				int newClientSocket = accept(masterSocket, NULL, NULL);

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

						// Send the HELLO packet to the player:
						SpacewarMessage helloMessage;
						helloMessage.type = 0;
						helloMessage.content = index;
						send(newClientSocket, &helloMessage, sizeof(SpacewarMessage), 0);

						// Add the player to the simulation:
						uint32_t secret = addPlayer(&connectedClients[index], index, currentState);

						// Send the SECRET packet to the player:
						helloMessage.type = 4;
						helloMessage.content = secret;
						send(newClientSocket, &helloMessage, sizeof(SpacewarMessage), 0);
						
						break;
					}
				}
			}
			// Otherwise, we've been sent a packet from one of the connected clients:
			else
			{
				SpacewarConnection * client = getConnectionBySocket(connectedClients, 32,
																	receivedEvents->data.fd);

				SpacewarMessage receivedMessage;
				size_t bytesRead = recv(client->clientSocket, &receivedMessage,
										sizeof(SpacewarMessage), 0);

				if (bytesRead == 0)
				{
					// Send a goodbye message:
					SpacewarMessage goodbyeMessage;
					goodbyeMessage.type = 1;
					goodbyeMessage.content = 0;
					send(client->clientSocket, &goodbyeMessage, sizeof(SpacewarMessage), 0);

					// Remove the socket from the epoll interest set:
					epoll_ctl(epollDescriptor, EPOLL_CTL_DEL, client->clientSocket, NULL);
					
					// Remove the player from the simulation:
					//removePlayer(&connectedClients[index], currentState);
					
					// Shutdown the socket:
					shutdown(client->clientSocket, SHUT_RDWR);

					// Deactivate the connection:
					client->active = false;
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
