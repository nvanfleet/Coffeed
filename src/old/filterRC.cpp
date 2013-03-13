#include "filterRC.h"

//--------------- FILTER RC

filterRC::filterRC() {
	level = 0;
	y = 0;
	first = 1;
};

// ----------------------------------------------------
void filterRC::init( int percent ) {
	level = percent;
	first = 1;
};

// ------------------------------------
int filterRC::execfilter ( int xi ) {
	if(first == 1) {
		y = xi;
		first = 0;
		return y;
	}
	float yy = (float)(100.0f - level) * (float)xi * 0.01f;
	float yyy = (float)level * (float)y * 0.01f;
	yy += yyy;
	
	y = (int) yy;
	
	return y; 
};