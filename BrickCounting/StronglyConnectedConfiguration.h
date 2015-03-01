#pragma once
#include "RectilinearBrick.h"

template <unsigned int SIZE>
class StronglyConnectedConfiguration
{
private:
	RectilinearBrick otherBricks[SIZE-1]; // first brick 0,0, vertical(angle 0) at lv. 0

public:
	StronglyConnectedConfiguration();
};

