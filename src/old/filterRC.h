#include <sys/types.h>

class filterRC {
public:
	filterRC();
	void init( int percent );
	int execfilter( int xi );
protected:
	int level; // filtering level, 0 to 100%
	int y; // most recent value of function
	bool first; // special handling of first call
};