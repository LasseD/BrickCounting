#ifndef UTIL_MPD_PRINTER_H
#define UTIL_MPD_PRINTER_H

#include <fstream>
#include <vector>

#include "LDRPrintable.h"

namespace util {
	class MPDPrinter {
	public:
		void print(const std::string &title) const;
		void add(const std::string &ldrName, const LDRPrintable *p);

	private:
		std::vector<std::pair<std::string, const LDRPrintable*> > sccs;

		void printHeader(std::ofstream &os, const std::string &title) const;
		void printFooter(std::ofstream &os) const;
	};
}

#endif // UTIL_MPD_PRINTER_H
