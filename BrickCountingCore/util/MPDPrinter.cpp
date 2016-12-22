#include "MPDPrinter.h"

#include <sstream>
#include <cmath>

#define MODEL_PADDING 13

namespace util {
	void MPDPrinter::add(const std::string &ldrName, const LDRPrintable *c) {
		blocks.push_back(std::make_pair(ldrName, c));
	}

	void MPDPrinter::print(const std::string &title) const {
		std::ofstream os;
		std::stringstream ss;
		ss << title << ".mpd";
		os.open(ss.str().c_str(), std::ios::out);

		int columns = (int)sqrt(blocks.size());
		int x = 0;
		int y = 0;
		printHeader(os, title);
		for (std::vector<std::pair<std::string, const LDRPrintable*> >::const_iterator it = blocks.begin(); it != blocks.end(); ++it, ++x) {
			if (x >= columns) {
				x = 0;
				++y;
			}
			os << "1 0 " << (x * 20 * MODEL_PADDING) << " 0 " << (y * 20 * MODEL_PADDING) << " 0 0 1 0 1 0 -1 0 0 " << it->first << ".ldr" << std::endl;
		}
		printFooter(os);

		// Print sub models:
		for (std::vector<std::pair<std::string, const LDRPrintable*> >::const_iterator it = blocks.begin(); it != blocks.end(); ++it) {
			printHeader(os, it->first);
			it->second->toLDR(os, 4);
			printFooter(os);
		}

		os << std::endl;
		os.close();
	}

	void MPDPrinter::printHeader(std::ofstream &os, const std::string &title) const {
		os << "0 FILE " << title << ".ldr" << std::endl;
		os << "0 " << title << std::endl;
		os << "0 Name: " << title << ".ldr" << std::endl;
		os << "0 Author: BrickCounting" << std::endl;
		os << "0 Unofficial Model" << std::endl;
		os << "0 ROTATION CENTER 0 0 0 1 \"Custom\"" << std::endl;
		os << "0 ROTATION CONFIG 0 0" << std::endl;
	}

	void MPDPrinter::printFooter(std::ofstream &os) const {
		os << "0 " << std::endl;
	}
}
