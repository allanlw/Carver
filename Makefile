#DEBUG=-DDEBUG
#DEBUG+=-pg
DEBUG+=-g

#OPT+=-O1
OPT+=-O2
OPT+=-finline-functions
#OPT+=-fno-inline
#OPT+=-fno-inline-functions-called-once 
OPT+=-funswitch-loops

CCFLAGS=-std=c++0x -Wall -Wextra $(OPT) $(DEBUG)

CCFILES=energy.cc frame.cc test.cc diff.cc
OFILES=$(patsubst %.cc,%.o,$(CCFILES))

all : test

test : $(OFILES)
	g++ $(CCFLAGS) $^ -o $@

%.o : %.cc
	g++ $(CCFLAGS) -c -o $@ $^

clean :
	rm -rf $(OFILES) test
