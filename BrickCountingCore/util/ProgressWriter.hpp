#ifndef UTIL_PROGRESS_WRITER_HPP
#define UTIL_PROGRESS_WRITER_HPP

#include <time.h>
#include <iostream>
#include <string>

#define SECONDS_BETWEEN_PROGRESS_REPORTS 10

namespace util {
	class ProgressWriter {
		time_t startTime, lastReportTime;
		unsigned long steps, step;
		std::string reportName;

		void outputTime(unsigned long time) {
			bool first = true;
			if (time > 3600 * 24) { // Days:
				std::cout << time / (3600 * 24) << " days";
				first = false;
				time %= 3600 * 24;
			}
			if (time > 3600 || !first) { // Hours:
				if (!first)
					std::cout << ", ";
				std::cout << time / (3600) << " hours";
				first = false;
				time %= 3600;
			}
			if (time > 60 || !first) { // Minutes:
				if (!first)
					std::cout << ", ";
				std::cout << time / (60) << " minutes";
				first = false;
				time %= 60;
			}
			if (!first)
				std::cout << ", ";
			std::cout << time << " seconds";
		}

	public:
		void initReportName(std::string name) {
			this->reportName = name;
		}
		void initSteps(unsigned long _steps) {
			this->steps = _steps;
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
			if (shouldDisplay) {
				unsigned long timeElapsed = (unsigned long)difftime(currTime, startTime);
				unsigned long timeLeft = ((steps - step) * timeElapsed) / step;
				int percentageDone = (int)(step * 100 / steps);

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

#endif // UTIL_PROGRESS_WRITER_HPP
