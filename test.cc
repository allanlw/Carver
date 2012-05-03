#include <iostream>
#include <fstream>
#include <string>
#include <getopt.h>

#include "frame.h"
#include "energy.h"
#include "diff.h"

using namespace std;

static const string default_ifilename = "frame.pnm";
static const string default_ofilename = "frame_carved.pnm";
static const string default_odebugfilename = "frame_seam.pnm";
static const bool default_debug = false;
static const size_t default_numcarves = 1;

void write_out(FrameWrapper& frame, string name) {
  cout << "Writing to " << name << "\n";
  writePnm(frame, name);
  cout << "Done writing output.\n";
}

FrameWrapper* read_in(string name) {
  cout << "Loading " << name << "\n";
  FrameWrapper* inputImage = loadPnm(name);
  if (inputImage == NULL) {
    cout << "Failed to load " << name << "\n";
  } else {
    cout << "Loaded " << name << " (" << inputImage->getHeight();
    cout << "x" << inputImage->getWidth();
    cout << " color:" << ((inputImage->color)?"true":"false") << ")" << "\n";
  }
  return inputImage;
}

void print_help() {
  cout << "\t-h\tPrint this help.\n";
  cout << "\t-o\tSpecify output filename (default: ";
  cout << default_ofilename << ")\n";
  cout << "\t-f\tSpecify input filename (default: ";
  cout << default_ifilename << ")\n";
  cout << "\t-d\tEnable debug outut (default: ";
  cout << (default_debug?"true":"false") << ")\n";
  cout << "\t-g\tSpecify debug output filename (default: ";
  cout << default_odebugfilename << ")\n";
  cout << "\t-c\tSpecify number of carves (default: ";
  cout << default_numcarves << ")\n";
  return;
}

int main(int argc, char** argv) {
  string ifilename = default_ifilename;
  string ofilename = default_ofilename;
  string odebugfilename = default_odebugfilename;
  bool debug = default_debug;
  size_t carves = default_numcarves;
  int c;

  while ((c = getopt(argc, argv, "f:o:dg:c:h")) != -1) {
    switch (c) {
    case 'h':
      print_help();
      return 0;
    case 'f':
      ifilename = optarg;
      break;
    case 'o':
      ofilename = optarg;
      break;
    case 'd':
      debug = true;
      break;
    case 'g':
      odebugfilename = optarg;
      break;
    case 'c':
      carves = atoi(optarg);
      break;
    default:
      return 1;
      break;
    }
  }

  FrameWrapper* inputImage = NULL;
  FrameWrapper* current = NULL;
  FrameWrapper* cut = NULL;

  inputImage = read_in(ifilename);

  if (inputImage == NULL) return 1;

  current = inputImage;

  for (size_t i = 0; i < carves; i++) {
    Frame<unsigned char>* diffF = getDifferential(*current);

    cout << "Calculating best flow...\n";
    FlowState* s = getBestFlow(*diffF, FLOW_LEFT_RIGHT);
    cout << "Done calculating best flow (" << s->points.size();
    cout << " nodes, flow: " << s->s.flow << ")!\n";

    cout << "Cutting frame...\n";
    if (debug) {
      cut = new FrameWrapper(current->color);
      cut->setSize(current->getWidth(), current->getHeight());
    } else {
      cut = NULL;
    }
    FrameWrapper* result = cutFrame(*s, *current, cut);
    delete current;
    current = result;
    delete diffF;
    delete s;
    cout << "Done cutting frame...\n";
  }

  if (debug) {
    write_out(*cut, odebugfilename);
  }

  write_out(*current, ofilename);

  delete current;
  delete cut;
  return 0;
}
