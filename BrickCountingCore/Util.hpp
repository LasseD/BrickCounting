#ifndef UTIL_HPP
#define UTIL_HPP

#include <time.h>
#include <iostream>
#include <string>

#define SECONDS_BETWEEN_PROGRESS_REPORTS 10

namespace util {
  template <typename T, unsigned int CAPACITY>
  class TinyVector {
  private:
    T elements[CAPACITY];
    unsigned int s;

  public:
    TinyVector() : s(0) {}

    TinyVector(const TinyVector& v) : s(v.s) {
      for(unsigned int i = 0; i < s; ++i)
        elements[i] = v.elements[i];
    }

    TinyVector& operator=(const TinyVector &v) {
      s = v.s;
      for(unsigned int i = 0; i < s; ++i)
        elements[i] = v.elements[i];
      return *this;
    }

    inline T& operator[](std::size_t pos) {
      assert(pos >= 0);
      assert(pos < s);
      return elements[pos];
    }
    inline const T& operator[](std::size_t pos) const { 
      assert(pos >= 0);
      assert(pos < s);
      return elements[pos]; 
    };

    inline unsigned int size() const {
      return s;
    }

    T const * begin() const {
      return &elements[0];
    }
    T const * end() const {
      return &elements[s];
    }

    T * begin() {
      return &elements[0];
    }
    T * end() {
      return &elements[s];
    }

    inline bool empty() const {
      return s == 0;
    }

    inline void clear() {
      s = 0;
    }
    inline void truncate(unsigned int size) {
      assert(size >= 0);
      assert(size <= s);
      s = size;
    }
    inline void pop_back() {
      assert(s > 0);
      --s;
    }

    inline void push_back(const T &element) {
      assert(s < CAPACITY);
      elements[s++] = element;
      if(s > CAPACITY) {
        int* die = NULL;
        std::cerr << "TinyVector over capacity! " << CAPACITY << std::endl;
        die[666] = 666;
      }
    }
  };

  class ProgressWriter {
    time_t startTime, lastReportTime;
    unsigned long steps, step;
    std::string reportName;

    void outputTime(unsigned long time) {
      bool first = true;
      if(time > 3600 * 24) { // Days:
        std::cout << time/(3600*24) << " days";
        first = false;
        time %= 3600*24;
      }
      if(time > 3600 || !first) { // Hours:
        if(!first)
          std::cout << ", ";
        std::cout << time/(3600) << " hours";
        first = false;
        time %= 3600;
      }
      if(time > 60 || !first) { // Minutes:
        if(!first)
          std::cout << ", ";
        std::cout << time/(60) << " minutes";
        first = false;
        time %= 60;
      }
      if(!first)
        std::cout << ", ";
      std::cout << time << " seconds";
    }

  public:
    void initReportName(std::string name) {
      this->reportName = name;
    }
    void initSteps(unsigned long steps) {
      this->steps = steps;
      step = 0;
    }
    void initTime() {
      time(&startTime);
      time(&lastReportTime);
    }

    void reportProgress() {
      ++step;
      time_t currTime;
      time(&currTime);

      unsigned long diffLastReport = (unsigned long)difftime(currTime, lastReportTime);
      bool shouldDisplay = diffLastReport >= SECONDS_BETWEEN_PROGRESS_REPORTS;
      if(shouldDisplay) {
        unsigned long timeElapsed = (unsigned long)difftime(currTime, startTime);
        unsigned long timeLeft = ((steps-step) * timeElapsed) / step;
        int percentageDone = (int)(step*100/steps);

        lastReportTime = currTime;
        std::cout << reportName << ": " << percentageDone << "% done after ";
        outputTime(timeElapsed);
        std::cout << ". Estimated remaining: ";
        outputTime(timeLeft);
        std::cout << "." << std::endl;
      }
    }
  };
}

#endif // UTIL_HPP
