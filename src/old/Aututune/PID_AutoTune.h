#ifndef PID_AutoTune
#define PID_AutoTune

class PID_ATune
{
public:
    PID_ATune();                       					// * Constructor.  links the Autotune to a given PID
    int update();						   			   	// * Similar to the PID Compute function, returns non 0 when done
	void cancel();									   	// * Stops the AutoTune
	
	void setOutputStep(int);						   	// * how far above and below the starting value will the output step?
	int getOutputStep();							   	//
	
	void setControlType(int); 						   	// * Determies if the tuning parameters returned will be PI (D=0)
	int getControlType();							   	//   or PID.  (0=PI, 1=PID)
	
	void setLookbackSec(int);							// * how far back are we looking to identify peaks
	
	void setNoiseBand(float);							// * the autotune will ignore signal chatter smaller than this value
	float getNoiseBand();								//   this should be acurately set
	
	float getKp();										// * once autotune is complete, these functions contain the
	float getKi();										//   computed tuning parameters.
	float getKd();										//
	
private:
	// Flags
	int justchanged;
	int running;
	int isMax, isMin;
	int lookbackptr;
	
	// Values
	int nLookBack;
	int peakType;
	int controlType;
	int peakCount;
	int oStep;
	int outputStart;
	
	long peak1, peak2;
	
	float setpoint;
	float noiseBand;
	float lastInputs[101];
    float peaks[10];
	float absMax, absMin;
	float Ku, Pu;
	
	void finishUp();
};
#endif

