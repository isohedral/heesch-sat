INCLUDES = -I/usr/local/include/cryptominisat5 -I/usr/local/include -I/opt/local/include

CC = clang
CPP = clang++
OPT = -O3  -DNDEBUG
# OPT = -g
CPPFLAGS = $(INCLUDES) $(OPT) -Wall -std=c++17  -stdlib=libc++ \
	-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk 

GRIDS = grid.h ominogrid.h hexgrid.h iamondgrid.h octasquaregrid.h draftergrid.h kitegrid.h

LIBS = -L/usr/local/lib -lcryptominisat5

all: sat

sat: main.o 
	$(CPP) $(LIBS) -o sat main.o

surrounds: surrounds.o 
	$(CPP) $(LIBS) -o surrounds surrounds.o

isohedral: isohedral.o 
	$(CPP) $(LIBS) -o isohedral isohedral.o

main.o: main.cpp cloud.h common.h geom.h heesch.h holes.h shape.h $(GRIDS)
	$(CPP) $(CPPFLAGS) -c -o main.o main.cpp

surrounds.o: surrounds.cpp cloud.h common.h geom.h heesch.h holes.h shape.h $(GRIDS)
	$(CPP) $(CPPFLAGS) -c -o surrounds.o surrounds.cpp

isohedral.o: isohedral.cpp cloud.h common.h geom.h heesch.h holes.h shape.h $(GRIDS)
	$(CPP) $(CPPFLAGS) -c -o isohedral.o isohedral.cpp

clean:
	rm *.o sat
