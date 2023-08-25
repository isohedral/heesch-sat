INCLUDES = -I/usr/local/include/cryptominisat5 -I/usr/local/include -I/opt/local/include -I/opt/local/include/cairo

CXX = clang++
OPT = -g
# OPT = -O3  -DNDEBUG
CXXFLAGS = -std=c++17 -Wall -MMD $(INCLUDES) $(OPT) -stdlib=libc++ \
	-isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk 
LIBS = -L/usr/local/lib -lcryptominisat5

OBJECTS = sat.o viz.o surrounds.o gen.o report.o
DEPENDS = ${OBJECTS:.o=.d}

all: sat viz gen surrounds report

sat: sat.o 
	$(CXX) $(LIBS) -o sat sat.o

viz: viz.o
	$(CXX) $(LIBS) -L/opt/local/lib -lcairo -o viz viz.o 

gen: gen.o
	$(CXX) $(LIBS) -o gen gen.o

surrounds: surrounds.o 
	$(CXX) $(LIBS) -o surrounds surrounds.o

report: report.o 
	$(CXX) $(LIBS) -o report report.o

# tile: tile.o 
#	$(CXX) $(LIBS) -o tile tile.o

-include ${DEPENDS}

.PHONY: clean

clean:
	rm ${OBJECTS} ${DEPENDS}
