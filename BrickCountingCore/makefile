# define some Makefile variables for the compiler and compiler flags
# to use Makefile variables later in the Makefile: $()
#
#  -g    adds debugging information to the executable file
#  -Wall turns on most, but not all, compiler warnings
#
# for C++ define  CC = g++
CC = g++
#CFLAGS = -Wall -D_TRACE=1 -D_INFO=1 -D_DEBUG=1 -DDEBUG=1
#CFLAGS = -Wall -D_INFO=1 -D_DEBUG=1 -DDEBUG=1
CFLAGS = -Wall -O2 -DNDEBUG

# typing 'make' will invoke the first target entry in the file 
# (in this case the default target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
default: bc

# To create the executable file bc we need the object files
#
bc:  MPDPrinter.o ConnectionPoint.o RectilinearBrick.o Math.o Brick.o StronglyConnectedConfigurationManager.o ConfigurationEncoder.o UnionFind.o TurningSingleBrick.o AngleMapping.o SingleConfigurationManager.o ConfigurationManager.o main.o
	$(CC) $(CFLAGS) -o bc MPDPrinter.o ConnectionPoint.o RectilinearBrick.o Math.o Brick.o StronglyConnectedConfigurationManager.o ConfigurationEncoder.o UnionFind.o TurningSingleBrick.o AngleMapping.o SingleConfigurationManager.o ConfigurationManager.o main.o

# To create the object file ??.o, we need the source
# files ??.h, and ??.cpp:
#
MPDPrinter.o:  util/MPDPrinter.h util/MPDPrinter.cpp
	$(CC) $(CFLAGS) -c util/MPDPrinter.cpp

ConnectionPoint.o: modelling/ConnectionPoint.h modelling/ConnectionPoint.cpp
	$(CC) $(CFLAGS) -c modelling/ConnectionPoint.cpp

Math.o:  Util.hpp Math.h Math.cpp
	$(CC) $(CFLAGS) -c Math.cpp

RectilinearBrick.o: util/MPDPrinter.h modelling/ConnectionPoint.h modelling/RectilinearBrick.h modelling/RectilinearBrick.cpp
	$(CC) $(CFLAGS) -c modelling/RectilinearBrick.cpp

Brick.o:  MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h Brick.cpp
	$(CC) $(CFLAGS) -c Brick.cpp

UnionFind.o: Brick.h UnionFind.h UnionFind.cpp
	$(CC) $(CFLAGS) -c UnionFind.cpp

StronglyConnectedConfigurationManager.o:  MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h StronglyConnectedConfigurationManager.cpp
	$(CC) $(CFLAGS) -c StronglyConnectedConfigurationManager.cpp

ConfigurationEncoder.o: MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h Configuration.hpp ConfigurationEncoder.h ConfigurationEncoder.cpp
	$(CC) $(CFLAGS) -c ConfigurationEncoder.cpp

TurningSingleBrick.o: MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h Configuration.hpp ConfigurationEncoder.h UnionFind.h TurningSingleBrick.h TurningSingleBrick.cpp
	$(CC) $(CFLAGS) -c TurningSingleBrick.cpp

AngleMapping.o:  MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h Configuration.hpp ConfigurationEncoder.h UnionFind.h TurningSingleBrick.h AngleMapping.h AngleMapping.cpp
	$(CC) $(CFLAGS) -c AngleMapping.cpp

SingleConfigurationManager.o:  MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h Configuration.hpp ConfigurationEncoder.h TurningSingleBrick.h AngleMapping.h SingleConfigurationManager.h SingleConfigurationManager.cpp
	$(CC) $(CFLAGS) -c SingleConfigurationManager.cpp

ConfigurationManager.o:  MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h Configuration.hpp ConfigurationEncoder.h TurningSingleBrick.h AngleMapping.h SingleConfigurationManager.h ConfigurationManager.h ConfigurationManager.cpp
	$(CC) $(CFLAGS) -c ConfigurationManager.cpp

main.o:  MPDPrinter.h ConnectionPoint.h RectilinearBrick.h Util.hpp Math.h Brick.h StronglyConnectedConfiguration.hpp StronglyConnectedConfigurationList.hpp StronglyConnectedConfigurationManager.h Configuration.hpp ConfigurationEncoder.h TurningSingleBrick.h AngleMapping.h SingleConfigurationManager.h ConfigurationManager.h main.cpp
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


