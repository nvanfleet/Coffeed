
/*
#define GPIO_GET        536887818
#define GPIO_SET        536887819
#define GPIO_CLEAR      536887820
#define GPIO_DIR_IN     536887821
#define GPIO_DIR_OUT    536887822
*/

/*
 MUST BE REVERSED FOR SOME REASON
 If you used gpioctl to set the pin it would trigger
 the boiler and vice-versa. For some reason it has to 
 be the opposite in program.
 
 Might have something to do with the fact I haven't 
 been able to successfully include the gpio_dev.h and
 had to get this from elsewhere.
*/

#define HEATEROFF GPIO_CLEAR
#define HEATERON GPIO_SET

#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "Heater.h"

//#include <linux/gpio_dev.h>
#include "gpio_dev.h"
#include "config.h"
#include "Logger.h"

extern Configuration conf;
extern Logger logger;

void Heater::setup() 
{	
	active = FALSE;
	cycle = 100; // 100 Hz
	duty = 0;
	_gpio_status = HEATEROFF;
    
	if ((gpio_bus = open("/dev/gpio", O_RDWR)) < 0)
	{
		 printf("/dev/gpio open Failure\n");
	}
	else
	{
		ioctl(gpio_bus, GPIO_DIR_OUT, conf.heat_pin);
		elementPower(HEATEROFF);
	}
}

Heater::Heater()
{
    
}

void Heater::update(long int time) 
{
	if(!active)
		return;

	float tPoint = (time % 1000 % cycle);

	// INSIDE HEAT CYCLE
	if(tPoint < duty)
		elementPower(HEATERON);
	else
		elementPower(HEATEROFF);
	
	fprintf(stderr,"TIME %li TP %f stat %d ON %d\n",time,tPoint,getDuty(),(_gpio_status==HEATERON));
}

void Heater::off()
{
	active = FALSE;
	elementPower(HEATEROFF);
}

void Heater::on()
{
	active = TRUE;
}

void Heater::setDuty( float d ) 
{
	// Power is 1 - 1000 for some reason
	d = d/10;
	
	duty = (int) d;
    
	if (duty < 0)
	{
		duty = 0;
		off();
		return;
	}
	else if (duty >= 100)
		duty = 100;

	on();
}

int Heater::getDuty()
{
    return duty;
}

void Heater::elementPower(int set) 
{
	if(_gpio_status != set)
	{
		_gpio_status=set;
		ioctl(gpio_bus, set, conf.heat_pin);
	}
}

void Heater::shutdown()
{
    elementPower(HEATEROFF);
    close(gpio_bus);
}

Heater::~Heater () 
{
    shutdown();
}
