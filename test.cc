#include "energy.h"
#include "frame.h"
#include "getopt.h"

#include <iostream>
#include <fstream>
#include <string>

#include "frame.h"
#include "energy.h"
#include "diff.h"

using namespace std;

template<class T>
void write_out_file(const Frame<T>& img, string name) {
  cout << "Writing to " << name << "\n";
  fstream ofile(name.c_str(), fstream::out);
  printPnm(img, ofile);
  ofile.close();
  cout << "Done writing output.\n";
}

int main(int argc, char** argv) {
  string ifilename = "frame.pgm";
  string ofilename = "frame_carved.pgm";
  string odebugfilename = "frame_seam.pgm";
  bool debug = false;
  int c;

  while ((c = getopt(argc, argv, "f:o:dg:")) != -1) {
    switch (c) {
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
    default:
      return 1;
      break;
    }
  }

  bool grey = false;
  union {
    Frame<unsigned char>* greyF;
    Frame<RgbPixel>* rgbF;
  };
  union {
    Frame<unsigned char>* greyR;
    Frame<RgbPixel>* rgbR;
  };
  union {
    Frame<unsigned char>* greyC;
    Frame<RgbPixel>* rgbC;
  };

  Frame<unsigned char>* diffF = NULL;

  cout << "Loading " << ifilename << "\n";
  fstream ifile(ifilename.c_str(), fstream::in);
  string magic = getMagic(ifile);
  if (magic == "P2" || magic == "P5") {
    grey = true;
    greyF = loadPgm(ifile);
  } else if (magic == "P3" || magic == "P6") {
    grey = false;
    rgbF = loadPpm(ifile);
  }
  ifile.close();
  if ((grey && greyF == NULL) || (!grey && rgbF == NULL)) {
    cout << "Failed to load " << ifilename << "\n";
    return 1;
  }
  cout << "Loaded " << ifilename << " (" << (grey?greyF->w:rgbF->w);
  cout << "x" << (grey?greyF->h:rgbF->w);
  cout << " grey:" << grey << ")" << "\n";

  diffF = (grey?getDifferential(*greyF):getDifferential(*rgbF));

  cout << "Calculating best flow...\n";
  FlowState* s = getBestFlow(*diffF, FLOW_LEFT_RIGHT);
  cout << "Done calculating best flow (" << s->points.size();
  cout << " nodes, flow: " << s->s.flow << ")!\n";

  cout << "Cutting frame...\n";
  if (debug) {
    if (grey) {
      greyC = new Frame<unsigned char>();
      greyC->w = greyF->w;
      greyC->h = greyF->h;
      greyC->values.resize(greyC->w * rgbC->h);
    } else {
      rgbC = new Frame<RgbPixel>();
      rgbC->w = rgbF->w;
      rgbC->h = rgbF->h;
      rgbC->values.resize(rgbC->w * rgbC->h);
    }
  } else {
    if (grey) {
      greyC = NULL;
    } else {
      rgbC = NULL;
    }
  }
  if (grey) {
    greyR = cutFrame(*s, *greyF, greyC);
  } else {
    rgbR = cutFrame(*s, *rgbF, rgbC);
  }
  cout << "Done cutting frame...\n";

  if (debug) {
    if (grey) {
      write_out_file(*greyC, odebugfilename);
    } else {
      write_out_file(*rgbC, odebugfilename);
    }
  }

  if (grey) {
    write_out_file(*greyR, ofilename);
  } else {
    write_out_file(*rgbR, ofilename);
  }

  delete s;
  if (grey) {
    delete greyF;
    delete greyR;
    delete greyC;
  } else {
    delete rgbF;
    delete rgbR;
    delete rgbC;
  }
  delete diffF;
  return 0;
}
