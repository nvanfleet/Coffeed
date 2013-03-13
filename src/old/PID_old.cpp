/*
 *  PID.cpp
 *  
 *
 *  Created by Nathan McDavitt-Van Fleet on 11-02-11.
 *  Copyright 2011 Logic Pretzel. All rights reserved.
 *
 */

#include "PID.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "Logger.h"

// Max accumulated error (Iterm)
#define WINDUP_GUARD_GAIN 100

// Reset iState if moving faster than this in the right direction
#define RESET_I_VELOCITY 0.007

extern Configuration conf;
extern Logger logger;

void PID::setup() 
{    
	logger.info("PID setup");
	iState = 0;
	lastTemp = 0;
}

PID::PID()
{
}

//IN CELCIUS
float PID::result(float targetTemp, float curTemp)
{
	double pTerm;
	double iTerm; 
	double dTerm; 	
	
	double error;
	double windupGaurd;
	
	// determine how badly we are doing
	error = targetTemp - curTemp;
	
	// the pTerm is the view from now, the pgain judges 
	// how much we care about error we are this instant.
	pTerm = conf.pgain * error;
	
	// iState keeps changing over time; it's 
	// overall "performance" over time, or accumulated error
	iState += error;
	
	// Modification to prevent overshoots
	// Reset iState if we are moving in the right direction
	// ToDo: Maybe decrease iState when moving in the right direction instead of reset
	// More decrease when moving faster etc.
	if((error > 0 && (curTemp - lastTemp) > RESET_I_VELOCITY) ||
       (error < 0 && (lastTemp - curTemp) > RESET_I_VELOCITY) ||
       (error < 0.02 && error > -0.02))
	{
		iState = 0;
	}
    
	// to prevent the iTerm getting huge despite lots of 
	//  error, we use a "windup guard" 
	// (this happens when the machine is first turned on and
	// it cant help be cold despite its best efforts)
	
	// not necessary, but this makes windup guard values 
	// relative to the current iGain
	windupGaurd = WINDUP_GUARD_GAIN / conf.igain;
	
	if (iState > windupGaurd) 
		iState = windupGaurd;
	else if (iState < -windupGaurd)
		iState = -windupGaurd;
    
	iTerm =  conf.igain * iState;
	
	// the dTerm, the difference between the temperature now
	//  and our last reading, indicated the "speed," 
	// how quickly the temp is changing. (aka. Differential)
	dTerm = ( conf.dgain * (curTemp - lastTemp));
	
	// now that we've use lastTemp, put the current temp in
	// our pocket until for the next round
	lastTemp = curTemp;
	
	// the magic feedback bit
	return  pTerm + iTerm - dTerm;
}
