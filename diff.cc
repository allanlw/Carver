#include "diff.h"

#include <cstdlib>

using namespace std;

Frame<unsigned char>* getDifferential(const Frame<unsigned char>& frame) {
  Frame<unsigned char>* result = new Frame<unsigned char>();
  result->w = frame.w;
  result->h = frame.h;
  for (size_t y = 0; y < frame.h; y++) {
    for (size_t x = 0; x < frame.w; x++) {
      if (x == frame.w-1 || y == frame.h-1) {
        result->values.push_back(255);
      } else {
        int v = abs(frame.values[(x+1)+frame.w*y] -
                    frame.values[x+frame.w*y]) +
                abs(frame.values[x+frame.w*(y+1)] -
                    frame.values[x+frame.w*y]);
        result->values.push_back(v);
      }
    }
  }
  return result;
}
Frame<unsigned char>* getDifferential(const Frame<RgbPixel>& frame) {
  return NULL;
}
