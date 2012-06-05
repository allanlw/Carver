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
#include "diff.h"

using namespace std;

Frame<PixelValue>* getDifferential(const Frame<PixelValue>& frame) {
  Frame<PixelValue>* result = new Frame<PixelValue>(frame.w, frame.h);
  for (size_t y = 0; y < frame.h; y++) {
    for (size_t x = 0; x < frame.w; x++) {
      if (x == frame.w-1 || y == frame.h-1) {
        result->values[x + y * frame.w] = 0xff;
      } else {
        int v = abs(frame.values[(x+1)+frame.w*y] -
                    frame.values[x+frame.w*y]) +
                abs(frame.values[x+frame.w*(y+1)] -
                    frame.values[x+frame.w*y]);
        result->values[x + y * frame.w] = (PixelValue)v;
      }
    }
  }
  return result;
}

Frame<PixelValue>* getDifferential(const Frame<RgbPixel>& frame) {
  Frame<PixelValue>* result = new Frame<PixelValue>(frame.w, frame.h);
  for (size_t y = 0; y < frame.h; y++) {
    for (size_t x = 0; x < frame.w; x++) {
      if (x == frame.w-1 || y == frame.h-1) {
        result->values[x + y * frame.w] = 0xff;
      } else {
        int rv = abs(frame.values[(x+1)+frame.w*y].r -
                     frame.values[x+frame.w*y].r) +
                 abs(frame.values[x+frame.w*(y+1)].r -
                     frame.values[x+frame.w*y].r);
        int gv = abs(frame.values[(x+1)+frame.w*y].g -
                     frame.values[x+frame.w*y].g) +
                 abs(frame.values[x+frame.w*(y+1)].g -
                     frame.values[x+frame.w*y].g);
        int bv = abs(frame.values[(x+1)+frame.w*y].b -
                     frame.values[x+frame.w*y].b) +
                 abs(frame.values[x+frame.w*(y+1)].b -
                     frame.values[x+frame.w*y].b);
        result->values[x + y * frame.w] = (PixelValue)max(rv, max(gv, bv));
      }
    }
  }
  return result;
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
  frame.values[x + frame.w * y] ^= one;
}

void togglePixel(FrameWrapper& frame, std::size_t x, std::size_t y) {
  if (frame.color) {
    togglePixel(*frame.colorFrame, x, y);
  } else {
    togglePixel(*frame.greyFrame, x, y);
  }
}
