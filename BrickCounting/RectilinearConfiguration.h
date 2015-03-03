#ifndef STRONGLY_CONNECTED_CONFIGURATION_H
#define STRONGLY_CONNECTED_CONFIGURATION_H

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

#endif
