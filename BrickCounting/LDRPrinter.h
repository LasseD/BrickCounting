#ifndef LDR_PRINTER_H
#define LDR_PRINTER_H

#include <fstream>
#include <vector>

#define LDR_COLOR_RED 4
#define LDR_COLOR_YELLOW 14
#define LDR_COLOR_BLACK 0
#define LDR_COLOR_BLUE 1

class LDRPrintable; // Forward declaration

class MPDPrinter {
public:
  MPDPrinter() {}
  void print(const std::string &title) const;
  void add(const LDRPrintable *p);

private:
  std::vector<const LDRPrintable*> sccs;
  void printHeader(std::ofstream &os, const std::string &title) const; 
  void printFooter(std::ofstream &os) const; 
};

class LDRPrintable {
public:
  virtual void toLDR(std::ofstream &os, int ldrColor) const = 0;
//  virtual void ldrName(std::ostream &ss) const = 0;
};

class MPDPrinterHandler {
public:
  MPDPrinterHandler() {}
  void print(const std::string &title) const;
  void add(const LDRPrintable *p);

private:
  std::vector<const LDRPrintable*> sccs;
  void printHeader(std::ofstream &os, const std::string &title) const; 
  void printFooter(std::ofstream &os) const; 
};

#endif // LDR_PRINTER_H
