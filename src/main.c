// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

/* ------------------------------------------------------------------------- */
/*       coffeed -- A PID system for espresso machines                       */
/* ------------------------------------------------------------------------- */

/*	
 
 A PID daemon for monitoring and controlling the heat of a espresso machine.
 
 */

/* ------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>

#include "config.h"
#include "Logger.h"
#include "PID.h"
#include "Sensor.h"
#include "Server.h"
#include "Heater-pwm.h"

#pragma mark - Setup Code

Configuration conf;
State state;
Heater heater;
Server server;
Sensor sensor;
PID pid;

pthread_t serverThread;

#pragma mark - Save config

void writeConfiguration(const char* cmd, float value)
{
	int bs = 128;
	char b[bs];
	
	// Special uci end command
	if(!strcasecmp(cmd,"commit"))
		snprintf(b, bs-1, "/sbin/uci %s",cmd);
	else
		snprintf(b, bs-1, "/sbin/uci set coffeed.%s=%.2f",cmd,value);
	
	FILE* pipe = popen(b, "r");
    
    if (!pipe) 
    	logger.info("error %s",cmd);
    
    pclose(pipe);
}

void updateConfiguration()
{
	if(conf.brewPoint != 0.0f && conf.steamPoint != 0.0f)
	{
		logger.info("Saving configuration");
		writeConfiguration("main.brew_setpoint",conf.brewPoint);
		writeConfiguration("main.steam_setpoint",conf.steamPoint);
		writeConfiguration("main.boiler_offset",conf.offset);
		writeConfiguration("main.p_const",pid.getP());
		writeConfiguration("main.i_const",pid.getI());
		writeConfiguration("main.d_const",pid.getD());
		writeConfiguration("commit",0);
	}
	else 
		logger.info("Not updating configuration");
}

#pragma mark - Setup code

void printhelp()
{
    printf("\n    Hardware\n--------------\n"
     "     -t hex          Ambient cold junction chip i2c address\n"
     "     -T hex          ADC thermocouple chip i2c address\n"
     "     -c int          ADC thermocouple chip channel\n"
     "     -i string       i2c_dev bus location ex: 0\n"
     "     -w string       PWM bus number ex: 0\n"
     "     -u int          Hz for how often to update      (max 4 times a second)\n"
     "\n    Server\n--------------\n"
     "     -l string       Log file location     '/tmp/pidd.log'\n"
     "     -L int          Logging ON/OFF        0 = FALSE will log to file 1 = TRUE will log to std out\n"
     "     -p int          Server Port           default 4949\n"
     "\n    PID\n--------------\n"
     "     -P float        P gain\n"
     "     -I float        I gain\n"
     "     -D float        D gain\n"
     "\n    Temperature\n--------------\n"
     "     -C int          Use Celcius = 1, use Farenheit = 0 for the below temp sets\n"
     "     -B float        Brew setpoint (degrees)\n"
     "     -S float        Steam setpoint (degrees)\n"
     "     -O float        Boiler offset value to estimate water temperature (degrees)\n"
	);
}

float convertToC(float tempF)
{
    return (tempF-32)*1.8;
}

void setup_config(int argc, char **argv)
{
	// Default values

	int use_c = TRUE;
	
	conf.logging = FALSE;
	
	conf.amb_add = 0x4b; // MCP9800 3A
	conf.adc_add = 0x68; // MCP3422 0A
	conf.adc_chan = 0;
	conf.i2c_bus = 0;
	conf.pwm_bus = 0;
	
	pid.setP(40.0f);
	pid.setI(30.0f);
	pid.setD(0.0f);
	
	conf.update = 4; // 4 times a second
	
	conf.brewPoint = 0.0f;
	conf.steamPoint = 0.0f;
	conf.offset = 0.0f;
	
	snprintf(conf.tmp_log, 20-1, "%s", "/tmp/coffeed.log");
	
	conf.port = 4949;

	// CLI input
	int opt;
	int val = 0;
	
	//INCLUDED ARGUMENTS FROM CLI
	while((opt = getopt(argc, argv, "t:T:c:i:w:l:u:p:L:P:I:D:C:B:S:O:")) > 0) 
	{
		switch(opt)
		{
			case 't':
				sscanf(optarg, "%x", &val);
				conf.amb_add = val;
				break;
			case 'T':
				sscanf(optarg, "%x", &val);
				conf.adc_add = val;
				break;
			case 'c':
				conf.adc_chan = atoi(optarg);
				break;
			case 'i':
				conf.i2c_bus = atoi(optarg);
				break;
			case 'w':
				conf.pwm_bus = atoi(optarg);
				break;
			case 'l':
				snprintf(conf.tmp_log, 20-1, "%s", (char*) optarg);
				break;
			case 'u':
				conf.update = atoi(optarg);
				break;
			case 'p':
				conf.port = atoi(optarg);
				break;
			case 'L':
				conf.logging = atoi(optarg);
				break;
			case 'P':
				pid.setP(atof(optarg));
				break;
			case 'I':
				pid.setI(atof(optarg));
				break;
			case 'D':
				pid.setD(atof(optarg));
				break;	
			case 'C':
				use_c = atoi(optarg);
				break;
			case 'B':
				conf.brewPoint = atof(optarg);
				break;
			case 'S':
				conf.steamPoint = atof(optarg);
				break;
			case 'O':
				conf.offset = atof(optarg);
				break;

			default:
                fprintf(stderr, "Check the config file '/etc/config/coffeed' and run with '/etc/init.d/coffeed start'\n Usage: %s Invalid\n",argv[0]);
                printhelp();
				exit(0);
		}
    }
	
    //Is Farenheit?
    if(use_c == FALSE)
    {
        conf.brewPoint = convertToC(conf.brewPoint);
        conf.steamPoint = convertToC(conf.steamPoint);
        conf.offset = convertToC(conf.offset);
    }
}

void setup_state()
{
	state.run = TRUE;
	state.brewmode = TRUE;
	state.active = FALSE;
	state.tempPoint = 0.0f;
	state.pidResult = 0;
	state.power = 0;
	state.pterm = 0.0f;
	state.iterm = 0.0f;
	state.dterm = 0.0f;
	state.time = 0;
}

static void sigHandle (int sig, siginfo_t *siginfo, void *context)
{
	logger.info("Handled signal: %d",sig);
	updateConfiguration();
	server.shutdown();
	heater.shutdown();
	sensor.shutdown();
	state.run = FALSE;
}

void setup_sigs()
{
	// SIGnal handling
	struct sigaction act;	
	memset (&act, '\0', sizeof(act));
	act.sa_flags = SA_SIGINFO;
	act.sa_sigaction = &sigHandle;

	sigaction(SIGHUP, &act, NULL);
	sigaction(SIGINT, &act, NULL);
	sigaction(SIGABRT, &act, NULL);
	sigaction(SIGSEGV, &act, NULL);
	sigaction(SIGTERM, &act, NULL);
	sigaction(SIGKILL, &act, NULL);
	sigaction(SIGQUIT, &act, NULL);
}

#pragma mark - Application code

void timeTick()
{
	struct timeval time;
	gettimeofday(&time, NULL);
	state.time = (clock_t) time.tv_sec*1000 + time.tv_usec/1000;
}

void *serverPoll( void *ptr )
{
	while(state.run == TRUE)
	{
		server.update();
	}
	return NULL;
}

int main(int argc, char **argv)
{
	// Setup
	setup_config(argc,argv);
	setup_state();
	setup_sigs();

	// Objects	
	logger.setup();
	server.setup();
	sensor.setup();
	heater.setup();

    // Thread
	if(pthread_create( &serverThread, NULL, serverPoll, NULL) != 0)
		logger.fail("Server thread creation failure");

	float chosenSetPoint;
	int sensorResult = 0;
	clock_t nextLoop;
	
	conf.update = 1000/conf.update;
	
	timeTick();
	nextLoop = state.time + conf.update;
    
 	logger.info("Starting up Coffeed");
 	
	while(state.run)
	{
		// Time update
		timeTick();

		if(state.time >= nextLoop)
		{
			// Get sensor data
			sensorResult = sensor.update();
		
			// If active choose set point
			if(state.active)
			{
				if(state.brewmode == TRUE)
					chosenSetPoint = conf.brewPoint;
				else
					chosenSetPoint = conf.steamPoint;
								
				// Sensor fault
				if(!sensorResult)
				{
					logger.fail("Sensor read failure");
					state.active = FALSE;
					heater.off();
				}
				// Regular control
				else
				{					
					// PID result
					state.pidResult = pid.result(chosenSetPoint, state.tempPoint);
					
					// Set Heater Duty
					heater.setPower( state.pidResult );
					
					// Log
					logger.debug("PID Result %d temperature %f setP %f brewP %f steamP %f", state.pidResult, state.tempPoint, chosenSetPoint, conf.brewPoint, conf.steamPoint);
				}
			}
			// Inactive system
			else
			{
				pid.reset();
				
				if(state.brewmode)
					state.brewmode = TRUE;
				
				chosenSetPoint = 0;
				heater.off();
			}

			// Get Heater Duty
			// This should be centralized to the heater
			state.power = heater.getPower();
			
			// Schedule the next loop after the whole operation	
			nextLoop = state.time + conf.update;
		}
		else
		{
			// Sleep until we need you
			usleep(1000 * (nextLoop - state.time));
		}
	}
	
    logger.info("Shutting down Coffeed");
}

