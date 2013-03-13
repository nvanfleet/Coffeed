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
#include <string.h>

#include "config.h"
#include "Logger.h"

void PID::setup() 
{    
	logger.info("PID setup");
	lastTemp = 0;
	error_ptr = 0;
	memset(pid_error, 0, PTR*sizeof(int));
}

PID::PID()
{
}

//IN CELCIUS
int PID::result(float targetTemp, float curTemp)
{
	float error;
	int out;
	int factor = 3;
	
	error = targetTemp - curTemp;
	
	error_ptr++;
	if(error_ptr > sizeof(pid_error)-1)
		error_ptr = 0;

	sum_errors -= pid_error[error_ptr];
	pid_error[error_ptr] = error;
	sum_errors += error;

	// the state.pterm is the view from now, the pgain judges 
	// how much we care about error we are this instant.
	//state.pterm = conf.pgain * error;
	state.pterm = (double) error/conf.pgain*100;
	
	//state.iterm =  conf.igain * iState;
	state.iterm = (double) (conf.igain * sum_errors)/conf.pgain;
	
	// the state.dterm, the difference between the temperature now
	//  and our last reading, indicated the "speed," 
	// how quickly the temp is changing. (aka. Differential)
	//state.dterm = ( conf.dgain * (curTemp - lastTemp));
	state.dterm = (double) conf.dgain * (curTemp-lastTemp)/conf.pgain;

	// now that we've use lastTemp, put the current temp in
	// our pocket until for the next round
	lastTemp = curTemp;
	
	out = (state.pterm + state.iterm - state.dterm)/factor;

	logger.debug("temp %f error %f errorsum %f state.pterm %f state.iterm %f state.dterm %f OUT %d",curTemp,error,sum_errors,state.pterm,state.iterm,state.dterm,out);

	if(out > 100)
		out = 100;
	else if(out < 0)
		out = 0;

	return out;
}
