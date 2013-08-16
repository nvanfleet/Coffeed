// Copyright (C) 2013 Nathan Van Fleet
//
// This is free software, licensed under the GNU General Public License v2.
// See /LICENSE for more information.

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
	float lastTemp;
	    
public:
    PID();
    void setup();
	int result(float targetTemp, float curTemp);
	void reset();
	void setP(float pv);
	void setI(float iv);
	void setD(float dv);
	float getP();
	float getI();
	float getD();
};

extern PID pid;

#endif
