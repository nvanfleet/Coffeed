// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

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

#define GAIN_MULTIPLIER 0.250f

void PID::setup() 
{    
	logger.info("PID setup");
	lastTemp = 0;
}

PID::PID()
{
}

float PID::getP()
{
	return conf.pgain;
}

float PID::getI()
{
	return conf.igain / GAIN_MULTIPLIER;
}

float PID::getD()
{
	return conf.dgain * GAIN_MULTIPLIER;
}

void PID::setP(float pv)
{
	conf.pgain = pv;
}

void PID::setI(float iv)
{
	conf.igain = iv * GAIN_MULTIPLIER;
}

void PID::setD(float dv)
{
	conf.dgain = dv / GAIN_MULTIPLIER;
}

void PID::reset()
{
	state.pterm = 0;
	state.pterm = 0;
	state.pterm = 0;
}

//IN CELCIUS
int PID::result(float targetTemp, float curTemp)
{
	int out;
	float error;
//	double windupGaurd;
	
	error = targetTemp - curTemp;

	// PTERM
	state.pterm = conf.pgain * error;
	
	// ITERM
	state.iterm += conf.igain * error;
	
	// Iterm sanity reset
	if (state.iterm > 100)
		state.iterm = 100;
	else if(state.iterm < 0)
		state.iterm = 0;
	
	// Modification to prevent overshoots
	// Reset iState if we are moving in the right direction
	// ToDo: Maybe decrease iState when moving in the right direction instead of reset
	// More decrease when moving faster etc.
//	if((abs(error) > 0 && (curTemp - lastTemp) > RESET_I_VELOCITY) ||
//       (error < 0.02 && error > -0.02))
//	{
//		state.iterm = 0;
//	}
	
	// to prevent the iTerm getting huge despite lots of
	//  error, we use a "windup guard"
	// (this happens when the machine is first turned on and
	// it cant help be cold despite its best efforts)
	
	// not necessary, but this makes windup guard values
	// relative to the current iGain
//	windupGaurd = WINDUP_GUARD_GAIN / conf.igain;
	
//	if (state.iterm > windupGaurd)
//		state.iterm = windupGaurd;
//	else if (state.iterm < -windupGaurd)
//		state.iterm = -windupGaurd;
	
	// the state.dterm, the difference between the temperature now
	//  and our last reading, indicated the "speed," 
	// how quickly the temp is changing. (aka. Differential)
	state.dterm = conf.dgain * (curTemp - lastTemp);

	// now that we've use lastTemp, put the current temp in
	// our pocket until for the next round
	lastTemp = curTemp;
	
	out = (state.pterm + state.iterm - state.dterm);

	logger.debug("temp %f error %f state.pterm %f state.iterm %f state.dterm %f OUT %d",curTemp,error,state.pterm,state.iterm,state.dterm,out);

	if(out > 100)
		out = 100;
	else if(out < 0)
		out = 0;

	return out;
}
