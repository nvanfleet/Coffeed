#ifndef SENSOR_89843215245
#define SENSOR_89843215245

#include <sys/types.h>
#include "TypeK.h"

//--------------- ADC

class ADC {
public:	
	int address;
	
	void setup(int add);
	int32_t read();
};

//--------------- AMBIENT

class Ambient {
private:
	int32_t fValue; 
	
	int32_t filter(uint32_t raw);
    
public:
	int address;
	float temperature;
	
	void setup(int add);
    void setConfig();
	void checkConfig();	
	int32_t read();
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
