INCLUDES = -I/usr/local/include/cryptominisat5 -I/usr/local/include -I/opt/local/include

CXX = clang++
OPT = -O3  -DNDEBUG
CXXFLAGS = -std=c++17 -Wall -MMD $(INCLUDES) $(OPT) -stdlib=libc++ \
	-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk 
LIBS = -L/usr/local/lib -lcryptominisat5
OBJECTS = sat.o viz.o

DEPENDS = ${OBJECTS:.o=.d}

all: sat viz

sat: sat.o 
	$(CXX) $(LIBS) -o sat sat.o

viz: viz.o
	$(CXX) $(LIBS) -o viz viz.o

bigpatch: bigpatch.o 
	$(CPP) $(LIBS) -o bigpatch bigpatch.o

surrounds: surrounds.o 
	$(CPP) $(LIBS) -o surrounds surrounds.o

isohedral: isohedral.o 
	$(CPP) $(LIBS) -o isohedral isohedral.o

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS} sat viz

# sat.o: sat.cpp cloud.h common.h geom.h heesch.h holes.h shape.h $(GRIDS)
# 	$(CPP) $(CPPFLAGS) -c -o sat.o sat.cpp

# surrounds.o: surrounds.cpp cloud.h common.h geom.h heesch.h holes.h shape.h $(GRIDS)
# 	$(CPP) $(CPPFLAGS) -c -o surrounds.o surrounds.cpp

# isohedral.o: isohedral.cpp cloud.h common.h geom.h heesch.h holes.h shape.h $(GRIDS)
# 	$(CPP) $(CPPFLAGS) -c -o isohedral.o isohedral.cpp

