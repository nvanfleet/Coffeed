/*
 *  Heater.h
 *  
 *
 *  Created by Nathan McDavitt-Van Fleet on 11-02-11.
 *  Copyright 2011 Logic Pretzel. All rights reserved.
 *
 */

#include <stdio.h>
#include <fcntl.h>

class Heater {
	int duty; // power value 1-100
	int cycle; // Hz value for updates
	int active;
    
	int _gpio_status;
	int gpio_bus;
	
	void elementPower(int set);
   	void on();
public:

 	Heater();
 	~Heater();
	void off();
	void setup();
	void update(long int time);
	void setDuty( float pwr );
    int getDuty();
	void shutdown();
};
