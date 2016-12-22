#ifndef UTIL_LDR_PRINTABLE_H
#define UTIL_LDR_PRINTABLE_H

#include <fstream>

#define LDR_COLOR_RED 4
#define LDR_COLOR_YELLOW 14
#define LDR_COLOR_BLACK 0
#define LDR_COLOR_BLUE 1
#define LDR_COLOR_GREEN 2
#define LDR_COLOR_ORANGE 25
#define LDR_COLOR_PURPLE 22

namespace util {
	class LDRPrintable {
	public:
		virtual void toLDR(std::ofstream &os, int ldrColor) const = 0;
	};
}

#endif // UTIL_LDR_PRINTABLE_H
