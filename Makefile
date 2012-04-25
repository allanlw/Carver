#DEBUG+=-pg
DEBUG+=-g
DEBUG+=-march=native

#OPT+=-O1
OPT+=-O2
#OPT+=-O3
#OPT+=-finline-functions
#OPT+=-fno-inline-small-functions
OPT+=-fshort-enums
#OPT+=-funsafe-loop-optimizations -Wunsafe-loop-optimizations
#OPT+=-fno-inline
#OPT+=-fno-inline-functions-called-once 
OPT+=-funswitch-loops
#OPT+=-fno-prefetch-loop-arrays
#OPT+=-fpredictive-commoning
#OPT+=-fgcse-after-reload
#OPT+=-ftree-vectorize
#OPT+=-fipa-cp-clone
OPT+=-fno-exceptions

CCFLAGS=-std=c++0x -Wall -Wextra $(OPT) $(DEBUG)

CCFILES=energy.cc frame.cc test.cc diff.cc
OFILES=$(patsubst %.cc,%.o,$(CCFILES))

all : test

test : $(OFILES)
#test : $(CCFILES)
	g++ $(CCFLAGS) $^ -o $@

%.o : %.cc
	g++ $(CCFLAGS) -c -o $@ $^

clean :
	rm -rf $(OFILES) test
