#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "Logger.h"
#include "Heater-pwm.h"
#include "PID_AutoTune.h"

PID_ATune::PID_ATune()
{
	running = FALSE;
	controlType = 0; //default to PI
	oStep = 1;
	noiseBand = 0.5f;
	setLookbackSec(10);
}

void PID_ATune::cancel()
{
	running = FALSE;
}

int PID_ATune::update()
{
	// Completed successfully
	if(peakCount > 9 && running)
	{
		logger.info("Success 2 - Autotune complete");
		running = FALSE;
		finishUp();
		return 1;
	}
	
	float refVal = state.tempPoint;
	
	// Begin
	if(running == FALSE)
	{
		//initialize working variables the first time around
		running = TRUE;
		justchanged = FALSE;
		peakType = 0;
		peakCount = 0;
		lookbackptr = 0;
		memset(lastInputs, 0, sizeof(int)*100);
		
		absMax = refVal;
		absMin = refVal;
		setpoint = refVal;
		outputStart = state.power;
		heater.setPower(outputStart + oStep);
	}
	else
	{
		if(refVal > absMax)
			absMax = refVal;
		
		if(refVal < absMin)
			absMin = refVal;
	}
	
	//oscillate the output base on the input's relation to the setpoint
	if(refVal > setpoint + noiseBand)
	{
		logger.info("down %d to %d",heater.getPower(), heater.getPower() - oStep);
		heater.setPower(heater.getPower() - oStep);
	}
	else if (refVal < setpoint - noiseBand)
	{
		logger.info("up %d to %d",heater.getPower(), heater.getPower() + oStep);
		heater.setPower(heater.getPower() + oStep);
	}
	
	// Bit resetting
	isMax = TRUE;
	isMin = TRUE;

	//id peaks
	for(int i = nLookBack-1; i >= 0; i--)
	{
		float val = lastInputs[i];
		
		if(isMax)
			isMax = refVal > val;
		
		if(isMin)
			isMin = refVal < val;
		
		lastInputs[i+1] = val;
	}
	
	lastInputs[0] = refVal;

	if(lookbackptr < nLookBack)
	{  //we don't want to trust the maxes or mins until the inputs array has been filled
		lookbackptr++;
		return FALSE;
	}
	
	if(isMax)
	{
		if(peakType == 0)
			peakType = 1;
		
		if(peakType == -1)
		{
			peakType = 1;
			justchanged = TRUE;
			peak2 = peak1;
		}
		
		peak1 = state.time;
		peaks[peakCount] = refVal;
	}
	else if(isMin)
	{
		if(peakType == 0)
			peakType = -1;
		
		if(peakType == 1)
		{
			peakType = -1;
			peakCount++;
			justchanged = TRUE;
			logger.info("new peak %d",peakCount);
		}
		
		if(peakCount<10)
			peaks[peakCount] = refVal;
	}
	
	//we've transitioned.  check if we can autotune based on the last peaks
	if(justchanged && peakCount > 2)
	{
		logger.info("checking peak");
		double avgSeparation = (abs(peaks[peakCount-1]-peaks[peakCount-2])+abs(peaks[peakCount-2]-peaks[peakCount-3]))/2;
		
		if(avgSeparation < 0.05*(absMax - absMin))
		{
			logger.info("Success 1");
			finishUp();
			running = FALSE;
			return TRUE;
		}
	}
	
	justchanged=FALSE;
	
	logger.info("no change");
	
	// We are not finished
	return FALSE;
}

void PID_ATune::finishUp()
{
	heater.setPower(outputStart);
	
	//we can generate tuning parameters!
	Ku = 4*(2*oStep)/((absMax-absMin)*3.14159);
	Pu = (float)abs(peak1 - peak2) / 1000;
}

float PID_ATune::getKp()
{
	return controlType==1 ? 0.6 * Ku : 0.4 * Ku;
}

float PID_ATune::getKi()
{
	return controlType==1? 1.2*Ku / Pu : 0.48 * Ku / Pu;  // Ki = Kc/Ti
}

float PID_ATune::getKd()
{
	return controlType==1? 0.075 * Ku * Pu : 0;  //Kd = Kc * Td
}

void PID_ATune::setOutputStep(int Step)
{
	oStep = Step;
}

int PID_ATune::getOutputStep()
{
	return oStep;
}

void PID_ATune::setControlType(int Type) //0=PI, 1=PID
{
	controlType = Type;
}
int PID_ATune::getControlType()
{
	return controlType;
}

void PID_ATune::setNoiseBand(float nband)
{
	noiseBand = nband;
}

float PID_ATune::getNoiseBand()
{
	return noiseBand;
}

void PID_ATune::setLookbackSec(int value)
{
    if (value<1) value = 1;
	
	if(value<25)
	{
		nLookBack = value * 4;
	}
	else
	{
		nLookBack = 100;
	}
}

