// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

/*
 *  Logger.cpp
 *  
 *
 *  Created by Nathan ; Fleet on 11-02-11.
 *  Copyright 2011 Logic Pretzel. All rights reserved.
 *
 */


#include <stdlib.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "Logger.h"
#include "config.h"

Logger logger;

void Logger::open_file()
{
    int fd, flags;
	logfile = fopen(conf.tmp_log, "a");
	if (logfile == NULL) 
    {
        printf("Failed to open log file %s\n",conf.tmp_log);
        exit(0);
	}
    
	/* Set the stream as line buffered */
	if (setvbuf(logfile, NULL, _IOLBF, 0) < 0)
    {
        printf("Failed to setvbuf\n");
        exit(0);
    }
	
	fd = fileno(logfile);
	flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Logger::setup()
{
    // Counters
    _fcnt = 0;
	
    // Open File
    open_file();
    
    info("-------------------");
    info("Logger setup");
}

Logger::Logger()
{
}

void Logger::log(const char *msg, const char* fmt, va_list &vl)
{
	size_t bsize = 0;
	int bufsize = 256;
	char buffer[bufsize];

	bsize = sprintf(buffer, msg);
	vsnprintf(&buffer[bsize], bufsize-bsize, fmt, vl);

	va_end(vl);
	
	if (conf.logging == FALSE)
		fprintf(stderr, "%s\n", buffer);
	else if(logfile)
		fprintf(logfile, "%s\n", buffer);
}

void Logger::fail(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr,fmt);
	log("ERROR: ", fmt, argptr);
}

// two different logging interfaces.
void Logger::info(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr,fmt);
	log("INFO: ", fmt, argptr);
}

// two different logging interfaces.
void Logger::data(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr,fmt);
	log("DATA: ", fmt, argptr);
}

void Logger::debug(const char* fmt, ...)
{
	va_list argptr;
	va_start(argptr,fmt);
	log("DEBUG: ", fmt, argptr);
}

void Logger::shutdown()
{
	info("Logger shutdown");
	fclose(logfile);
}
