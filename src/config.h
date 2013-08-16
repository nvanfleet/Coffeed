// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

#ifndef CONFIG_5635445367
#define CONFIG_5635445367

#include <sys/types.h>

#define TRUE    1
#define FALSE   0

/*
#define GPIO1 0
#define GPIO2 1
#define GPIO3 2
#define GPIO4 3
#define GPIO5 4
#define GPIO6 5 // DEAD PIN
#define GPIO7 6
#define GPIO8 7
*/

typedef struct {
	// System
	int run;
	int power;
	int active;
	int brewmode;
	int pidResult;
	long time;
	float tempPoint;
	float pterm;
	float iterm;
	float dterm;
} State;

typedef struct configuration {
    /* IMMUTABLE */
    // Pins
	int amb_add;
	int adc_add;
	int adc_chan;
    int i2c_bus;
	int pwm_bus;
	int logging;

	unsigned int port;
	
    // Timing/Logging
	long int update;
    
    // ALGORITHM
	float pgain;
	float igain;
	float dgain;
    
    // Temperature
	float brewPoint;
	float steamPoint;
	float offset;
	
    // Bus Logs Port
	char tmp_log[20];
} Configuration;

extern Configuration conf;
extern State state;

#endif
