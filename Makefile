#DEBUG+=-pg
DEBUG+=-g
#DEBUG+=-march=native

#OPT+=-O1
OPT+=-O2
#OPT+=-O3
#OPT+=-finline-functions
#OPT+=-fno-inline-small-functions
#OPT+=-fshort-enums
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
#OPT+=-ftree-parallelize-loops
#OPT+=-fopenmp
#OPT+=-fpack-struct -Wpacked

CCFLAGS=-Wall -Wextra $(OPT) $(DEBUG)
CCFLAGS+=-std=c++0x

INTERACTIVEFLAGS:=$(shell pkg-config gtkmm-3.0 --libs --cflags)

CCFILES=energy.cc frame.cc test.cc diff.cc interactive.cc
OFILES:=$(filter-out interactive.o, $(patsubst %.cc,%.o,$(CCFILES)))

EXEFILES=test interactive

TESTFILES:=$(OFILES)
INTERACTIVEFILES:=$(filter-out test.o, $(OFILES)) interactive.cc

all : $(EXEFILES)

interactive : $(INTERACTIVEFILES)
	g++ $(CCFLAGS) $^ -o $@ $(INTERACTIVEFLAGS)

test : $(TESTFILES)
	g++ $(CCFLAGS) $^ -o $@

%.o : %.cc
	g++ $(CCFLAGS) -c -o $@ $^

clean :
	rm -rf $(OFILES) $(EXEFILES)
