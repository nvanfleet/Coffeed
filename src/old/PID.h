/*
 *  PID.h
 *  
 *
 *  Created by Nathan McDavitt-Van Fleet on 11-02-11.
 *  Copyright 2011 Logic Pretzel. All rights reserved.
 *
 */

#ifndef PID_89843215245
#define PID_89843215245

#define PTR 10

class PID {

	float iState;
	float lastTemp;
	int pid_error[PTR];
	int error_ptr;
	int sum_errors;
    
public:
    PID();
    void setup();
	float result(float targetTemp, float curTemp);
};

#endif
