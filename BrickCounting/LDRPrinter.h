#ifndef LDR_PRINTER_H
#define LDR_PRINTER_H

#include <fstream>
#include <vector>

#define LDR_COLOR_RED 4
#define LDR_COLOR_YELLOW 14
#define LDR_COLOR_BLACK 0
#define LDR_COLOR_BLUE 1

class LDRPrintable {
public:
  virtual void toLDR(std::ofstream &os, int ldrColor) const = 0;
};

class MPDPrinter {
public:
  void print(const std::string &title) const;
  void add(const std::string &ldrName,const LDRPrintable *p);

private:
  std::vector<std::pair<std::string,const LDRPrintable*> > sccs;

  void printHeader(std::ofstream &os, const std::string &title) const; 
  void printFooter(std::ofstream &os) const; 
};

#endif // LDR_PRINTER_H
