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
#ifndef _FRAME_H
#define _FRAME_H

#include <cstdint>
#include <cstddef>
#include <vector>
#include <istream>
#include <ostream>
#include <string>

#include "const.h"

typedef std::uint8_t PixelValue;

template<typename T> struct Frame {
  typedef std::vector<T> ValuesSet;

  std::size_t w;
  std::size_t h;
  ValuesSet values;

  Frame() {}
  Frame(std::size_t w, std::size_t h) : w(w), h(h) {
    values.resize(w * h);
  }

  Frame<T>& operator=(const Frame<T>& other) {
    if (this != &other) {
      this->w = other.w;
      this->h = other.h;
      this->values = other.values;
    }
    return *this;
  }
};

struct RgbPixel {
  PixelValue r, g, b;

  RgbPixel() : r(0), g(0), b(0) { }

  RgbPixel& operator=(const RgbPixel& b) {
    if (this != &b) {
      this->r = b.r;
      this->g = b.g;
      this->b = b.b;
    }
    return *this;
  }

  RgbPixel& operator^=(const RgbPixel& b) {
    this->r ^= b.r;
    this->b ^= b.b;
    this->g ^= b.g;
    return *this;
  }
};

struct FrameWrapper {
  bool color;
  union {
    Frame<RgbPixel>* colorFrame;
    Frame<PixelValue>* greyFrame;
  };

  FrameWrapper() : color(true) { }

  FrameWrapper(const FrameWrapper& frame) : color(frame.color) {
    if (color) {
      colorFrame = new Frame<RgbPixel>;
      *colorFrame = *frame.colorFrame;
    } else {
      greyFrame = new Frame<PixelValue>;
      *greyFrame = *frame.greyFrame;
    }
  }

  FrameWrapper(bool color) : color(color) {
    if (color) {
      colorFrame = new Frame<RgbPixel>;
    } else {
      greyFrame = new Frame<PixelValue>;
    }
  }

  std::size_t getHeight() const {
    return color?colorFrame->h:greyFrame->h;
  }

  std::size_t getWidth() const {
    return color?colorFrame->w:greyFrame->w;
  }

  void setSize(std::size_t w, std::size_t h) {
    if (color) {
      colorFrame->h = h;
      colorFrame->w = w;
      colorFrame->values.resize(colorFrame->h * colorFrame->w);
    } else {
      greyFrame->h = h;
      greyFrame->w = w;
      greyFrame->values.resize(greyFrame->h * greyFrame->w);
    }
  }

  virtual ~FrameWrapper() {
    if (color) {
      delete colorFrame;
    } else {
      delete greyFrame;
    }
  }
};

std::string getMagic(std::istream& is);

Frame<PixelValue>* loadPgm(std::istream& is);
Frame<RgbPixel>* loadPpm(std::istream& is);
FrameWrapper* loadPnm(std::istream& is);

void printPgm(const Frame<PixelValue>& f, std::ostream& os,
              bool binary=PNM_BINARY_DEFAULT);
void printPpm(const Frame<RgbPixel>& f, std::ostream& os,
              bool binary=PNM_BINARY_DEFAULT);

void printPnm(const Frame<RgbPixel>& img, std::ostream& out,
              bool binary=PNM_BINARY_DEFAULT);
void printPnm(const Frame<PixelValue>& img, std::ostream& out,
              bool binary=PNM_BINARY_DEFAULT);
void printPnm(const FrameWrapper& img, std::ostream& out,
              bool binary=PNM_BINARY_DEFAULT);

void writePnm(const FrameWrapper& img, std::string name);
FrameWrapper* readPnm(std::string name);

#endif
