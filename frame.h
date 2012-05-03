#ifndef _FRAME_H
#define _FRAME_H

#include <cstddef>
#include <vector>
#include <istream>
#include <ostream>
#include <string>

#define _BINARY_DEFAULT true

template<typename T> struct Frame {
  typedef std::vector<T> ValuesSet;

  std::size_t w;
  std::size_t h;
  ValuesSet values;
};

struct RgbPixel {
  unsigned char r, g, b;

  RgbPixel() : r(0), g(0), b(0) { }

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
    Frame<unsigned char>* greyFrame;
  };

  FrameWrapper() : color(true) { }

  FrameWrapper(bool color) : color(color) {
    if (color) {
      colorFrame = new Frame<RgbPixel>;
    } else {
      greyFrame = new Frame<unsigned char>;
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

Frame<unsigned char>* loadPgm(std::istream& is);
Frame<RgbPixel>* loadPpm(std::istream& is);
FrameWrapper* loadPnm(std::istream& is);

void printPgm(const Frame<unsigned char>& f, std::ostream& os,
              bool binary=_BINARY_DEFAULT);
void printPpm(const Frame<RgbPixel>& f, std::ostream& os,
              bool binary=_BINARY_DEFAULT);

void printPnm(const Frame<RgbPixel>& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);
void printPnm(const Frame<unsigned char>& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);
void printPnm(const FrameWrapper& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);


void writePnm(const FrameWrapper& img, std::string name);
FrameWrapper* loadPnm(std::string name);

#endif
