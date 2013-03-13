#ifndef SENSOR_89843215245
#define SENSOR_89843215245

#include <sys/types.h>
#include "TypeK.h"
#include "filterRC.h"

//--------------- ADC

class ADC {
public:	
	int address;
	
	void setup(int add);
	int32_t read();
};

//--------------- AMBIENT

class Ambient {
    
	uint32_t filtered; // up to date filtered raw reading
	uint32_t raw; // most recent raw sensor reading
	int32_t v;
	
	filterRC filter;    

public:	
	int address;
	float ambC;
	float ambF;
	
	void setup(int add);
    void setConfig();
    void checkConfig();
	int32_t read();
	float getOffset(); // returns calibration information
	void setOffset( float tempC ); // allows override of default offset
};

//--------------- SENSOR

class Sensor {

private:
    Ambient amb;
	ADC adc;
	TypeK tc;
	int select(int add);
    
public:
    Sensor();
    ~Sensor();
    void setup();
	int update();
	void shutdown();
};

#endif
