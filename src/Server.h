// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

//
//  server.h
//  CoffeeD
//
//  Created by Nathan Van Fleet on 11-12-30.
//  Copyright (c) 2011 Logic Pretzel. All rights reserved.
//

#ifndef CoffeeD_server_h
#define CoffeeD_server_h

#include <netinet/in.h>

#define BUFSIZE 128

class Server {
private:
	int serversocket;
	int clientsocket;
	struct pollfd *pollfds;

	void parseData(char *rbuf, int rsize);
	int interpret(char *comm, char *value, int retval, char *response, int responseSize, int responseOffset);
	void respond(char *rbuf, int rsize);
	void closeConnection();
	
public:
	Server();
	~Server();
	void setup();

	void update();

	void shutdown();
};

#endif
