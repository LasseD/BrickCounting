#ifndef LDR_PRINTER_H
#define LDR_PRINTER_H

#include <fstream>
#include <vector>

#define LDR_COLOR_RED 4
#define LDR_COLOR_YELLOW 14
#define LDR_COLOR_BLACK 0

class LDRPrinter {
public:
  virtual void toLDR(std::ofstream &os, int x, int y, int ldrColor) const = 0;
};

class LDRPrinterHandler {
public:
  LDRPrinterHandler() {}
  void print(const std::string &title) const;
  void add(const LDRPrinter *p);

private:
  std::vector<const LDRPrinter*> sccs;
  void printHeader(std::ofstream &os, const std::string &title) const; 
  void printFooter(std::ofstream &os) const; 
};

#endif // LDR_PRINTER_H
