#include "LDRPrinter.h"

#include <sstream>
#include <cmath>

#define MODEL_PADDING 13

void LDRPrinterHandler::add(const LDRPrinter *c) {
  sccs.push_back(c);
}

void LDRPrinterHandler::print(const std::string &title) const {
  std::ofstream os;
  std::stringstream ss;
  ss << title << ".ldr";
  os.open(ss.str().c_str(), std::ios::out);

  printHeader(os, title);

  int columns = (int)sqrt(sccs.size());
  
  int x = 0;
  int y = 0;
  for(std::vector<const LDRPrinter*>::const_iterator it = sccs.begin(); it != sccs.end(); ++it, ++x) {
    if(x >= columns) {
      x = 0;
      ++y;
    }
    (*it)->toLDR(os, MODEL_PADDING*x, MODEL_PADDING*y, 4);
  }

  printFooter(os);
  os.close();
}

void LDRPrinterHandler::printHeader(std::ofstream &os, const std::string &title) const {
  os << "0 " << title << std::endl;
  os << "0 Name: " << title << ".ldr" << std::endl;
  os << "0 Author: BrickCounting, Lasse Deleuran (lassedeleuran@gmail.com)" << std::endl;
  os << "0 Unofficial Model" << std::endl;
  os << "0 ROTATION CENTER 0 0 0 1 \"Custom\"" << std::endl; 
  os << "0 ROTATION CONFIG 0 0" << std::endl;
}

void LDRPrinterHandler::printFooter(std::ofstream &os) const {
  os << "0 " << std::endl;
  os << std::endl;
}
