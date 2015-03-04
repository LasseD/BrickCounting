# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC = g++
CFLAGS = -Wall

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: bc

# To create the executable file bc we need the object files
#
bc:  RectilinearBrick.o StronglyConnectedConfigurationManager.o main.o
	$(CC) $(CFLAGS) -o bc RectilinearBrick.o StronglyConnectedConfigurationManager.o main.o

# To create the object file RectilinearBrick.o, we need the source
# files RectilinearBrick.h, and RectilinearBrick.cpp:
#
RectilinearBrick.o:  RectilinearBrick.h RectilinearBrick.cpp
	$(CC) $(CFLAGS) -c RectilinearBrick.cpp

StronglyConnectedConfigurationManager.o:  RectilinearBrick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h StronglyConnectedConfigurationManager.cpp
	$(CC) $(CFLAGS) -c StronglyConnectedConfigurationManager.cpp

main.o:  RectilinearBrick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h main.cpp
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


