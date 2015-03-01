#pragma once

#include "StronglyConnectedConfiguration.h"

template <unsigned int P>
class RectilinearConfiguration
{
private:
	int partsSizes[P]; // Decreasing 
	StronglyConnectedConfiguration parts[P];

public:
	RectilinearConfiguration();
};

