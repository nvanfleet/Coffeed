
/*
 MCSP432x CONFIG
 0 -- >	READY BIT
 1 -- \ CHANNEL	00-CHAN1 01-CHAN2
 2 -- /			10-CHAN3 11-CHAN3
 3 -- > 1-CONTINUOUS / 0-ONE-SHOT 
 4 -- \ SAMPLE RATE 00-12BITS 01-14BITS
 5 -- /				10-16BITS 11-18BITS
 6 -- \ PGA GAIN	00-x1 01-x2
 7 -- /				10-x4 11-x8
*/

// MCSP4322
#define A_CONT_G1	0b10011100
#define A_CONT_G2	0b10011101
#define A_CONT_G4	0b10011110
#define A_CONT_G8	0b10011111

#define A_SHOT_G1	0b10001100
#define A_SHOT_G2	0b10001101
#define A_SHOT_G4	0b10001110
#define A_SHOT_G8	0b10001111

/*
 MCP9800 CONFIG
 0 -- >	0-CONTINUOUS / 1-ONE-SHOT 
 1 -- \ RESOLUTION	00-09BIT 01-10BIT
 2 -- /				10-11BIT 11-12BIT
 3 -- \ FAULT QUEUE 00-1 01-2
 4 -- /				10-4 11-6
 5 -- > ALERT POL.	0-ACTHIGH 1-ACTLOW
 6 -- > 0-INTERRUPT 1-COMPARATOR
 7 -- > 0-SHUTDOWN
 */

#define A_TEMP_REG		0b00000000
#define A_CONFIG_REG	0b00000001

// Continuous
#define A_CONT_BITS9	0b00000000
#define A_CONT_BITS10	0b00100000
#define A_CONT_BITS11	0b01000000
#define A_CONT_BITS12	0b01100000

// One-shot mode
#define A_ONE_BITS9		0b10000001
#define A_ONE_BITS10	0b10100001
#define A_ONE_BITS11	0b11000001
#define A_ONE_BITS12	0b11100001

// CONFIGS
#define RES_12
#define AMB_CONFIG A_CONT_BITS12
#define ADC_GAIN 8
#define	ADC_CONFIG A_CONT_G8

#define BITS_TO_uV 15.625  // LSB = 15.625 uV

// ---------------------------- calibration of ADC and ambient temp sensor
#define CAL_OFFSET  ( 0 )  // microvolts
#define CAL_GAIN 1.00
#define AMB_LSB 0.0625 // value of MCP9800 LSB in 12-bit mode
#define AMB_FILTER 0.7f // 70% filtering on ambient sensor readings

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "i2c-dev.h"
//#include <linux/i2c.h>
//#include <linux/i2c-dev.h>

#include "Sensor.h"
#include "Logger.h"
#include "config.h"

//--------------- ADC

int i2c_bus_address;

void ADC::setup(int add)
{
   logger.info("ADC setup");
    
	address = add;
	if(i2c_smbus_write_byte(i2c_bus_address, ADC_CONFIG)<0)
        logger.fail("ADC setup failure");
}

int32_t ADC::read()
{
	uint8_t stat;
	uint8_t a, b, c, gain;
	int32_t v;
	float xv;
	
	uint8_t bytes = 4;
	uint8_t buf[bytes];
	
	if(i2c_smbus_read_i2c_block_data(i2c_bus_address, ADC_CONFIG, bytes, buf) < 0)
		logger.fail("ADC Block Read Failed");
	
	a = buf[0];
	b = buf[1];
	c = buf[2];
	stat = buf[3];
	gain = stat & 0b11;
	
	v = a;
	v <<= 24;
	v >>= 16;
	v |= b;
	v <<= 8;
	v |= c;
	
	// convert to microvolts
	xv = v;
	v = xv * BITS_TO_uV;
	
	// divide by gain
	v >>= gain;
	v *= CAL_GAIN;    // calibration of gain
    
//	logger.debug("ADC result %d raw %f gain %d",v,xv,gain);
    
	return v;
}

//--------------- AMBIENT

void Ambient::setConfig()
{
    if(i2c_smbus_write_byte_data(i2c_bus_address, A_CONFIG_REG, AMB_CONFIG) < 0)
        logger.fail("Ambient setup failure");
}

void Ambient::checkConfig()
{
    uint8_t bytes = 1;
	uint8_t buf[bytes];
	
	if(i2c_smbus_read_i2c_block_data(i2c_bus_address, A_CONFIG_REG, bytes, buf) < 0)
        	logger.fail("AMB Block Read Failed");
    
//	logger.data("AMB config %d",buf[0]);
//	logger.data("AMB config should be %d",AMB_CONFIG);
}

void Ambient::setup(int add)
{
	logger.info("AMB setup");
	fValue = -1;
	address = add;
	
	setConfig();
}

int32_t Ambient::filter(uint32_t raw)
{
	if(fValue == -1)
	{
		fValue = raw;
		return fValue;
	}
	
	float y, yy;
	
	y = (1.0f - AMB_FILTER) * (float) raw;
	yy = AMB_FILTER * (float) fValue;
	
	fValue =  (int32_t) (y + yy);
	
	return fValue; 
}

int32_t Ambient::read()
{
	int32_t raw,filtered;
	uint8_t bytes = 2;
	uint8_t buf[bytes];

	if(i2c_smbus_read_i2c_block_data(i2c_bus_address, A_TEMP_REG, bytes, buf) < 0)
		logger.fail("AMB Block Read Failed");
	
	uint32_t va = buf[0];
	uint32_t vb = buf[1];

	uint32_t result = (va << 8) | vb;
	
#ifdef RES_12
	// 12-bit code
	raw = result >> 4; // LSB = 0.0625C
#endif
#ifdef RES_11
	// 11-bit code
	raw = result >> 5; // LSB = 0.125C
	raw <<= 1;
#endif
#ifdef RES_10
	// 10-bit code
	raw = result >> 6; // LSB = 0.25C
	raw <<= 2;
#endif
#ifdef RES_9
	// 9-bit code
	raw = result >> 7; // LSB = 0.5C
	raw <<= 3;
#endif

	filtered = filter( raw );
	temperature = filtered * AMB_LSB;
	
//	logger.data("AMB buffers %d %d -> result %d -> raw %d -> filtered %d -> amb C %f ", va, vb, result, raw, filtered, temperature);

	return filtered;
}

//--------------- SENSOR

void Sensor::setup()
{
	logger.info("Sensor setup");

	tc = TypeK();
    
	int bs = 30;
	char buffer[bs];
	snprintf(buffer,bs-1,"/dev/i2c-%d",conf.i2c_bus);
	
	if ((i2c_bus_address = open(buffer, O_RDWR)) < 0)
        logger.fail("%s open Failure", buffer);
    
	// Disabled because of MCP9800 is not compatible
    // Activate error checking
//    if(ioctl(i2c_bus_address, I2C_PEC, 1) < 0)
//    	logger.fail("Error check enable failure");
    
	if(select(conf.amb_add) > 0)
		amb.setup(conf.amb_add);

	if(select(conf.adc_add) > 0)
		adc.setup(conf.adc_add);
}

Sensor::Sensor()
{
}

int Sensor::select(int add)
{
	if(ioctl(i2c_bus_address, I2C_SLAVE, add) < 0)
	{
		logger.fail("i2c select fail %d",add);
		if(ioctl(i2c_bus_address, I2C_SLAVE_FORCE, add) < 0)
		{
			logger.fail("i2c select FORCE fail %d",add);
			return -1;
		}
	}
	return 1;
}

int Sensor::update()
{
	int32_t adcReading;
	
	if(select(amb.address)>0)
		amb.read();
	else
	{		
		logger.fail("Bad Ambient reading");
		return FALSE;
	}
		
	if(select(adc.address)>0)
		adcReading = adc.read();
	else
	{		
		logger.fail("Bad ADC reading");
		return FALSE;
	}
		
	state.tempPoint = tc.Temp_C( 0.001 * adcReading , amb.temperature ) + conf.offset;

//    logger.debug("SENSOR: AmbC %5.3f Thermo %d", amb.temperature, adcReading);

	if(state.tempPoint > 200.0f || state.tempPoint < 0.0f)
	{
		logger.fail("Bad reading");
		return FALSE;
	}

    return TRUE;
}

void Sensor::shutdown()
{
	close(i2c_bus_address);
}

Sensor::~Sensor () 
{
	logger.info("Sensor shutdown");
	shutdown();
}
