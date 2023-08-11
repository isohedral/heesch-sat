INCLUDES = -I/usr/local/include/cryptominisat5 -I/usr/local/include -I/opt/local/include

CXX = clang++
OPT = -g
# OPT = -O3  -DNDEBUG
CXXFLAGS = -std=c++17 -Wall -MMD $(INCLUDES) $(OPT) -stdlib=libc++ \
	-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk 
LIBS = -L/usr/local/lib -lcryptominisat5
OBJECTS = sat.o viz.o surrounds.o

DEPENDS = ${OBJECTS:.o=.d}

all: sat viz surrounds

sat: sat.o 
	$(CXX) $(LIBS) -o sat sat.o

viz: viz.o
	$(CXX) $(LIBS) -o viz viz.o

surrounds: surrounds.o 
	$(CXX) $(LIBS) -o surrounds surrounds.o

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS} sat viz surrounds
