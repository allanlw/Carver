/* Copyright (C) 2012 Allan Wirth <allan@allanwirth.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
 * NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
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
  FrameWrapper* inputImage = readPnm(name);
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

  FlowState state(*current);

  for (size_t i = 0; i < carves; i++) {
    cout << "Calculating best flow...\n";
    state.calcBestFlow(FLOW_LEFT_RIGHT);
    cout << "Done calculating best flow (" << state.points.size();
    cout << " nodes, flow: " << state.s.flow << ")!\n";

    cout << "Cutting frame...\n";
    if (debug) {
      cut = new FrameWrapper(current->color);
      cut->setSize(current->getWidth(), current->getHeight());
    } else {
      cut = NULL;
    }
    FrameWrapper* result = state.cutFrame(*current, cut);
    delete current;
    current = result;
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
