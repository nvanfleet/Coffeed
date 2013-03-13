//
//  server.c
//  CoffeeD
//
//  Created by Nathan Van Fleet on 11-12-30.
//  Copyright (c) 2011 Logic Pretzel. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <netinet/tcp.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <math.h>

#include "Server.h"
#include "config.h"
#include "Logger.h"
#include "PID.h"

#define BUFSIZE 128

#pragma mark - Basic

void Server::setup()
{
	logger.info("Server setup");

	int z, b = 1;
 
	// Address
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port =  htons(conf.port);
	server_address.sin_addr.s_addr = ntohl(INADDR_ANY);
    
	// Create socket
	serversocket = socket(PF_INET,SOCK_STREAM,0);
	if ( serversocket == -1 )
		logger.fail("Server socket(2)");
    
	// SO_REUSEADDR
	z= setsockopt(serversocket,SOL_SOCKET, SO_REUSEADDR, &b, sizeof b);
	if ( z == -1 ) 
		logger.fail("Server setsockopt(2)");
    
	// Bind
	z= bind(serversocket, (struct sockaddr *)&server_address, sizeof (server_address));
	if ( z == -1 )
		logger.fail("Server bind(2)");
    
	// O_NONBLOCK
	//fcntl(serversocket, F_SETFL, O_NONBLOCK);
    
	// LISTEN
	z = listen(serversocket, 10);
	if(z == -1)
		logger.fail("listen(2)");
    
	// POLL
	pollfds = (struct pollfd*) malloc(sizeof(struct pollfd));
	if (pollfds == NULL)
		logger.fail("Server out of memory");
    
	pollfds[0].fd = serversocket;
	pollfds[0].events = POLLIN;
}

#pragma mark - connection

void Server::update()
{
	int ready = poll(pollfds, 1, 10);
	
	if (ready == -1)
	{
		logger.info("Poll failed");
		return;
	}
    
	// NEW CONNECTION
	if (pollfds[0].revents & POLLIN)
	{
		char charbuf[BUFSIZE];
		memset(charbuf, 0, BUFSIZE);
		
		ssize_t z;
		
		struct sockaddr_in client_address;
		memset(&client_address, 0, sizeof client_address);
		socklen_t alen = sizeof(client_address);
		
		// Accept connection
		clientsocket = accept(serversocket, (struct sockaddr *)&client_address, &alen);
		if(!clientsocket)
		{
			logger.fail("Accept Fail");
			return;
		}

		// Read data
		z = recv (clientsocket, charbuf, BUFSIZE, 0);
		if(z<0)
		{
			logger.fail("receive failed %d",z);
			return;
		}

		strcat(charbuf,",");
		
		logger.debug("Received command: %s",charbuf);
        
		// Parse which nests respond()
		parseData(charbuf, z);
		
		// Close connection
		closeConnection();
	}
}

int Server::interpret(char *comm, char *value, int scannum, char *response, int responseSize, int responseOffset)
{
	// READ ONLY
 	int success = TRUE;
	int readonly = FALSE;
	int boolValue = atoi(value);
	float floatValue = atof(value);
	int wasFloat = TRUE;
	
	char *responseString = (char *) malloc( sizeof(char) * ( 10 + 1 ) );
	
	if(scannum < 2)
	{
		readonly = TRUE;
		floatValue = -1.0f;
		boolValue = -1;
	}

	if(isnan(floatValue))
		floatValue = -1.0f;
	
	// BPOINT=X or BPOINT for readout
#pragma mark - Special
	if(!strcasecmp(comm,"VERSION"))
	{
		wasFloat = FALSE;
		sprintf(responseString,"2.5.0");
	}
	else if(!strcasecmp(comm,"TIME"))
	{
		wasFloat = FALSE;
		sprintf(responseString,"%li",state.time);
	}
	else if(!strcasecmp(comm,"SHUTD"))
	{
		wasFloat = FALSE;
		sprintf(responseString,"%d",1);
		state.run = FALSE;
	}
	else if(!strcasecmp(comm,"TPOINT"))
	{
		floatValue = state.tempPoint;
	}
	else if(!strcasecmp(comm,"POW"))
	{
		wasFloat = FALSE;
		sprintf(responseString,"%d",state.power);
	}
	else if(!strcasecmp(comm,"PTERM"))
	{
		floatValue = state.pterm;
	}
	else if(!strcasecmp(comm,"ITERM"))
	{
		floatValue = state.iterm;
	}
	else if(!strcasecmp(comm,"DTERM"))
	{
		floatValue = state.dterm;
	}
	else if(!strcasecmp(comm,"BMODE"))
	{
		wasFloat = FALSE;
		
		// WRITE
		if(readonly == FALSE)
			if(boolValue == TRUE)
				state.brewmode = TRUE;
			else
				state.brewmode = FALSE;
		
		sprintf(responseString,"%d",state.brewmode);
	}
	else if(!strcasecmp(comm,"SMODE"))
	{
		wasFloat = FALSE;
		
		// WRITE
		if(readonly == FALSE)
			if(boolValue == TRUE)
				state.brewmode = FALSE;
			else
				state.brewmode = TRUE;

		// READ
		if(state.brewmode == TRUE)
			sprintf(responseString,"%d",FALSE);
		else
			sprintf(responseString,"%d",TRUE);
	}
	else if(!strcasecmp(comm,"ACTIVE"))
	{
		wasFloat = FALSE;

		// WRITE
		if(readonly == FALSE)
			if(boolValue == TRUE)
				state.active = TRUE;
			else
				state.active = FALSE;
		
		sprintf(responseString,"%d",state.active);
	}
	else if(!strcasecmp(comm,"SLEEP"))
	{
		wasFloat = FALSE;
		
		// WRITE
		if(readonly == FALSE)
			if(boolValue == TRUE)
				state.active = FALSE;
			else
				state.active = TRUE;
		
		// READ
		if(state.active == TRUE)
			sprintf(responseString,"%d",FALSE);
		else
			sprintf(responseString,"%d",TRUE);
	}
#pragma mark - Float values
	else if(!strcasecmp(comm,"SETPOINT"))
	{
		// WRITE
		if(readonly == FALSE)
			if(state.brewmode == TRUE)
				conf.brewPoint = floatValue;
			else
				conf.steamPoint = floatValue;
		
		//READ
		if(state.brewmode == TRUE)
			floatValue = conf.brewPoint;
		else
			floatValue = conf.steamPoint;
	}
	else if(!strcasecmp(comm,"BPOINT"))
	{
		if(readonly != TRUE)
			conf.brewPoint = floatValue;
		else
			floatValue = conf.brewPoint;
	}
	else if(!strcasecmp(comm,"SPOINT"))
	{
		if(readonly != TRUE)
			conf.steamPoint = floatValue;
		else
			floatValue = conf.steamPoint;
	}
	else if(!strcasecmp(comm,"PGAIN"))
	{
		if(readonly != TRUE)
			pid.setP(floatValue);
		else
			floatValue = pid.getP();
	}
	else if(!strcasecmp(comm,"IGAIN"))
	{
		if(readonly != TRUE)
			pid.setI(floatValue);
		else
			floatValue = pid.getI();
	}
	else if(!strcasecmp(comm,"DGAIN"))
	{
		if(readonly != TRUE)
			pid.setD(floatValue);
		else
			floatValue = pid.getD();
	}
	else if(!strcasecmp(comm,"OFFSET"))
	{
		if(readonly != TRUE)
			conf.offset = floatValue;
		else
			floatValue = conf.offset;
	}
	else
		success = FALSE;
    
	if(success)
	{
		if(wasFloat == TRUE)
		{
			snprintf(&response[responseOffset], responseSize-responseOffset, "%s=%3.1f,\0",comm,floatValue);
		}
		else
		{
			snprintf(&response[responseOffset], responseSize-responseOffset, "%s=%s,\0",comm,responseString);
		}
	}
	else
	{
		snprintf(&response[responseOffset], responseSize-responseOffset, "%s=-1,\0",comm);
	}
	
	return strlen(response);
}

void Server::parseData(char *charbuf, int csize)
{
	int z = 0;
	int point = 0;
	int responseOffset = 0;
	char response[BUFSIZE];
	char segment[20];
	char cmd[10], val[10];
	char singlechar;
	
	for(int i=0; i<strlen(charbuf); i++)
	{
		if(charbuf[i] == ',')
		{
			snprintf(segment, i-point+1+1, "%s", &charbuf[point]);
			z = sscanf(segment, "%[^=,]=%[^,],", cmd, val);
			responseOffset = interpret(cmd, val, z, response, BUFSIZE, responseOffset);
			point = i+1;
		}
	}
	
	// Respond
	respond(response, responseOffset);
}

void Server::respond(char *response, int rsize)
{
	// SEND
	ssize_t z;
	
	z = send(clientsocket, response, rsize+1, 0);
	if (z < 0)
		logger.debug("Send Failure");
		
	logger.info("Sent message: %s",response);
}

void Server::closeConnection()
{
	close(clientsocket);
}

Server::Server()
{
}

void Server::shutdown()
{
	logger.info("Server shutdown");
	close(serversocket);
}

Server::~Server () 
{
	shutdown();
}
