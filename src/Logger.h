/*
 *  Logger.h
 *  
 *
 *  Created by Nathan McDavitt-Van Fleet on 11-02-11.
 *  Copyright 2011 Logic Pretzel. All rights reserved.
 *
 */

#ifndef LOGGER_354256029
#define LOGGER_354256029

#include <sys/types.h>
#include <stdarg.h>

#define TMP_SAMPLE_RATE 4 // 4 times a second for ten seconds 

//---------------------

class Logger {
    
    FILE *logfile;
    
    int _fcnt;
    
    //Funcs
    void open_file();
	void log(const char *msg, const char* fmt, va_list &vl);
public:
    
    Logger();
    void setup();
    void info( const char *logstring, ...);
    void data( const char *logstring, ...);
	void debug( const char *logstring, ...);
    void fail( const char *logstring, ...);
	void shutdown();
};

extern Logger logger;

#endif
