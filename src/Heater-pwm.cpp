// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "Heater-pwm.h"

#include "config.h"
#include "Logger.h"

#define BASIC_PERIOD 10000000

void Heater::writeToFile(const char *file, long unsigned int set)
{
	snprintf(&pwmbus[pwml],strlen(file)+1,"%s",file);

	FILE *pwmf = fopen(pwmbus, "w");
	if(pwmf != NULL)
	{
		if(fprintf(pwmf,"%lu",set) < 0)
			logger.fail("%s write failure", file);
		fclose(pwmf);
	}
	else
		logger.fail("%s open fail", file);	
}

void Heater::setActive(int set)
{		
	writeToFile("/run", set);
}

void Heater::setPolarity(int set)
{		
	writeToFile("/polarity", set);
}

void Heater::setPeriod(long unsigned int set)
{
	period = set;
	
	writeToFile("/period_ns", period);
}

void Heater::setDuty(long unsigned int set)
{
	writeToFile("/duty_ns", set);
}

void Heater::makeRequest()
{
	char buf[1];
	snprintf(&pwmbus[pwml],8+1,"/request");
	int fd = open(pwmbus, O_RDONLY); 
	read(fd, buf, 1); 
	close(fd);
}

void Heater::setPower( int newpower ) 
{
	if(newpower != power)
		power = newpower;
	else 
		return;
		
	// Set Duty
	long unsigned int newduty = 0;
	
	if(power <= 0)
	{
		newduty = 0;
		power = 0;	
	}
	else
		newduty = (period*power)/100.0f;
	
	setDuty(newduty);
}

int Heater::getPower()
{
    return power;
}

void Heater::setup() 
{
	logger.info("Heater setup (PWM)");

	// Preliminary bus setup
	snprintf(pwmbus,50-1,"/sys/class/pwm/gpio_pwm.0:%d",conf.pwm_bus);
	pwml = strlen(pwmbus);

	// Make Request
	makeRequest();
	
	// Set period
	setPeriod(BASIC_PERIOD);

	// Set polarity
	setPolarity(1);

	// Set Initial Duty
	setPower(0);
	
	// Set Active
	setActive(1);
}

Heater::Heater()
{
    
}

void Heater::off()
{
	setPower(0);
}

void Heater::shutdown()
{
    off();
    setActive(FALSE);
}

Heater::~Heater () 
{
    logger.info("Heater shutdown");
    shutdown();
}
