#ifndef LALG_H
#define LALG_H

#include "Lib/Laser/lalg_global.h"
#include "qvector.h"
class LALG_EXPORT LALG
{
public:
	LALG();
	~LALG();
	
	int calculate_result(double* raw_data, double* result, uint sf1_start, uint sf1_end, uint sf2_start, uint sf2_end);
private:
	
};

#endif // LALG_H
