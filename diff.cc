#include "diff.h"

#include <cstdlib>

using namespace std;

Frame<PixelValue>* getDifferential(const Frame<PixelValue>& frame) {
  Frame<PixelValue>* result = new Frame<PixelValue>();
  result->w = frame.w;
  result->h = frame.h;
  result->values.reserve(frame.w * frame.h);
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
Frame<PixelValue>* getDifferential(const Frame<RgbPixel>& frame) {
  return NULL;
}

Frame<PixelValue>* getDifferential(const FrameWrapper& frame) {
  if (frame.color) {
    return getDifferential(*frame.colorFrame);
  } else {
    return getDifferential(*frame.greyFrame);
  }
}

void zeroFrame(Frame<PixelValue>& frame) {
  for (size_t y = 0; y < frame.h; y++) {
    for (size_t x = 0; x < frame.w; x++) {
      frame.values[x + frame.w * y] = 0;
    }
  }
}

void zeroFrame(Frame<RgbPixel>& frame) {
  RgbPixel nil;
  nil.r = 0;
  nil.g = 0;
  nil.b = 0;

  for (size_t y = 0; y < frame.h; y++) {
    for (size_t x = 0; x < frame.w; x++) {
      frame.values[x + frame.w * y] = nil;
    }
  }
}

void zeroFrame(FrameWrapper& frame) {
  if (frame.color) {
    zeroFrame(*frame.colorFrame);
  } else {
    zeroFrame(*frame.greyFrame);
  }
}

void togglePixel(Frame<PixelValue>& frame, std::size_t x, std::size_t y) {
  frame.values[x + frame.w * y] ^= 0xff;
}

void togglePixel(Frame<RgbPixel>& frame, std::size_t x, std::size_t y) {
  RgbPixel one;
  one.r = 0xff;
  one.g = 0xff;
  one.b = 0xff;
  frame.values[x + frame.h * y] ^= one;
}

void togglePixel(FrameWrapper& frame, std::size_t x, std::size_t y) {
  if (frame.color) {
    togglePixel(*frame.colorFrame, x, y);
  } else {
    togglePixel(*frame.greyFrame, x, y);
  }
}
