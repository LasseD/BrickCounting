# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC = g++
CFLAGS = -Wall
#CFLAGS = -Wall -O2 -DNDEBUG

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: bc

# To create the executable file bc we need the object files
#
bc:  LDRPrinter.o RectilinearBrick.o Brick.o StronglyConnectedConfigurationManager.o RectilinearConfigurationManager.o main.o
	$(CC) $(CFLAGS) -o bc LDRPrinter.o RectilinearBrick.o Brick.o StronglyConnectedConfigurationManager.o RectilinearConfigurationManager.o main.o

# To create the object file ??.o, we need the source
# files ??.h, and ??.cpp:
#
LDRPrinter.o:  LDRPrinter.h LDRPrinter.cpp
	$(CC) $(CFLAGS) -c LDRPrinter.cpp

RectilinearBrick.o:  LDRPrinter.h RectilinearBrick.h RectilinearBrick.cpp
	$(CC) $(CFLAGS) -c RectilinearBrick.cpp

Brick.o:  LDRPrinter.h RectilinearBrick.h Brick.h Brick.cpp
	$(CC) $(CFLAGS) -c Brick.cpp

StronglyConnectedConfigurationManager.o:  LDRPrinter.h RectilinearBrick.h Brick.h StronglyConnectedConfiguration.hpp RectilinearConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h StronglyConnectedConfigurationManager.cpp
	$(CC) $(CFLAGS) -c StronglyConnectedConfigurationManager.cpp

RectilinearConfigurationManager.o:  LDRPrinter.h RectilinearBrick.h Brick.h StronglyConnectedConfiguration.hpp RectilinearConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h RectilinearConfigurationManager.h RectilinearConfigurationManager.cpp
	$(CC) $(CFLAGS) -c RectilinearConfigurationManager.cpp

main.o:  LDRPrinter.h RectilinearBrick.h Brick.h StronglyConnectedConfiguration.hpp RectilinearConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h RectilinearConfigurationManager.h main.cpp
	$(CC) $(CFLAGS) -c main.cpp

# To start over from scratch, type 'make clean'.  This
# removes the executable file, as well as old .o object
# files and *~ backup files:
#
clean: 
	$(RM) bc *.o *~

# build an executable named myprog from myprog.c
#all: main.cpp
#	g++ -Wall -o bc.exe main.cpp


