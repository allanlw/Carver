#ifndef _FRAME_H
#define _FRAME_H

#include <cstddef>
#include <vector>
#include <istream>
#include <ostream>
#include <string>
#include <cstdint>

#define _BINARY_DEFAULT true

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
              bool binary=_BINARY_DEFAULT);
void printPpm(const Frame<RgbPixel>& f, std::ostream& os,
              bool binary=_BINARY_DEFAULT);

void printPnm(const Frame<RgbPixel>& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);
void printPnm(const Frame<PixelValue>& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);
void printPnm(const FrameWrapper& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);

void writePnm(const FrameWrapper& img, std::string name);
FrameWrapper* loadPnm(std::string name);

#endif
