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

  RgbPixel& operator^=(const RgbPixel& b) {
    this->r ^= b.r;
    this->b ^= b.b;
    this->g ^= b.g;
    return *this;
  }
};

std::string getMagic(std::istream& is);

Frame<unsigned char>* loadPgm(std::istream& is);
Frame<RgbPixel>* loadPpm(std::istream& is);

void printPgm(const Frame<unsigned char>& f, std::ostream& os,
              bool binary=_BINARY_DEFAULT);
void printPpm(const Frame<RgbPixel>& f, std::ostream& os,
              bool binary=_BINARY_DEFAULT);

void printPnm(const Frame<RgbPixel>& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);
void printPnm(const Frame<unsigned char>& img, std::ostream& out,
              bool binary=_BINARY_DEFAULT);

#endif
