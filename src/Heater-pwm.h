/*
 *  Heater.h
 *  
 *
 *  Created by Nathan McDavitt-Van Fleet on 11-02-11.
 *  Copyright 2011 Logic Pretzel. All rights reserved.
 *
 */

#ifndef HEATER_89843215245
#define HEATER_89843215245

#include <stdio.h>

class Heater {
private:
	int power;
	long unsigned int period;
	int pwml;
	char pwmbus[50];
	
	void writeToFile(const char *file, long unsigned int set);
	void setActive(int set);
	void setPolarity(int set);
	void setPeriod(long unsigned int set);
	void setDuty(long unsigned int set);
	void makeRequest();
public:
	Heater();
 	~Heater();
	void setup();
	void off();
	void setPower(int newpower);
	int getPower();
	void shutdown();
};

extern Heater heater;

#endif
