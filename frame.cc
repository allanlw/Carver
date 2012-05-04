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
#include "frame.h"

#include <string>
#include <cctype>
#include <deque>
#include <fstream>

using namespace std;

FrameWrapper* loadPnm(istream& is) {
  string magic = getMagic(is);
  FrameWrapper* result = new FrameWrapper;
  if (magic == "P2" || magic == "P5") {
    result->color = false;
    result->greyFrame = loadPgm(is);
    if (result->greyFrame == NULL) return NULL;
  } else if (magic == "P3" || magic == "P6") {
    result->color = true;
    result->colorFrame = loadPpm(is);
    if (result->colorFrame == NULL) return NULL;
  } else {
    return NULL;
  }
  return result;
}

string getMagic(std::istream& is) {
  char c = is.get();
  char c2 = is.peek();
  is.unget();
  string res;
  res += c;
  res += c2;
  return res;
}

Frame<PixelValue>* loadPgm(std::istream& is) {
  Frame<PixelValue>* r = new Frame<PixelValue>();

  r->w = 0;
  r->h = 0;

  int max = -1;

  string line;

  getline(is, line);
  bool binary;
  if (line == "P5") {
    binary = true;
  } else if (line == "P2") {
    binary = false;
  } else {
    return NULL;
  }
  bool comment = false;
  bool raster = false;
  while (true) {
    int c = is.peek();
    if (is.eof()) break;
    if (binary && raster) {
      r->values.push_back(is.get() * 0xFF / max);
      if (r->values.size() > r->w * r->h) {
        delete r;
        return NULL;
      }
    } else if (!comment && c == '#') {
      comment = true;
      is.get();
    } else if (c == '\n') {
      comment = false;
      is.get();
    } else if (!comment && isdigit(c)) {
      unsigned int v;
      is >> v;
      if (r->w == 0) {
        r->w = v;
      } else if (r->h == 0) {
        r->h = v;
      } else if (max == -1) {
        max = v;
        r->values.reserve(r->w * r->h);
        if (binary) {
          raster = true;
          is.get(); // discard single whitespace before raster
        }
      } else {
        r->values.push_back(v * 0xFF / max);
        if (r->values.size() > r->w * r->h) {
          delete r;
          return NULL;
        }
      }
    } else {
      is.get();
    }
  }

  return r;
}

Frame<RgbPixel>* loadPpm(std::istream& is) {
  Frame<RgbPixel>* r = new Frame<RgbPixel>();

  string line;

  deque<unsigned int> values;

  getline(is, line);
  bool binary;
  if (line == "P6") {
    binary = true;
  } else if (line == "P3") {
    binary = false;
  } else {
    return NULL;
  }
  bool comment = false;
  bool raster = false;
  while (true) {
    int c = is.peek();
    if (is.eof()) break;
    if (binary && raster) {
      values.push_back(is.get());
    } else if (!comment && c == '#') {
      comment = true;
      is.get();
    } else if (c == '\n') {
      comment = false;
      is.get();
    } else if (!comment && isdigit(c)) {
      unsigned int v;
      is >> v;
      values.push_back(v);
      if (binary && values.size() == 3) {
        raster = true;
        is.get(); // discard 1xwhitespace value
      }
    } else {
      is.get();
    }
  }

  r->w = values.front();
  values.pop_front();
  r->h = values.front();
  values.pop_front();
  int max = values.front();
  values.pop_front();
  r->values.reserve(r->w * r->h);
  while (values.size()) {
    RgbPixel p;
    p.r = values.front() * 0xFF/max;
    values.pop_front();
    p.g = values.front() * 0xFF/max;
    values.pop_front();
    p.b = values.front() * 0xFF/max;
    values.pop_front();
    r->values.push_back(p);
  }

  return r;
}

void printPgm(const Frame<PixelValue>& f, ostream& os, bool binary) {
  os << (binary?"P5":"P2")<<"\n";
  os << f.w << " " << f.h << "\n";
  os << 255 << "\n";
  size_t j = 0;
  for (Frame<PixelValue>::ValuesSet::const_iterator i = f.values.begin();
       i != f.values.end(); ++i) {
    if (!binary) {
      os << (unsigned int)*i << " ";
      if (++j == 19) {
        os << "\n";
        j = 0;
      }
    } else {
      os << *i;
    }
  }
}

void printPpm(const Frame<RgbPixel>& f, ostream& os, bool binary) {
  os << (binary?"P6":"P3") << "\n";
  os << f.w << " " << f.h << "\n";
  os << 255 << "\n";
  size_t j = 0;
  for (Frame<RgbPixel>::ValuesSet::const_iterator i = f.values.begin();
       i != f.values.end(); ++i) {
    if (!binary) {
      os << (unsigned int)(*i).r << " ";
      if (++j == 19) {
        os << "\n";
        j = 0;
      }
      os << (unsigned int)(*i).g << " ";
      if (++j == 19) {
        os << "\n";
        j = 0;
      }
      os << (unsigned int)(*i).b << " ";
      if (++j == 19) {
        os << "\n";
        j = 0;
      }
    } else {
      os << (*i).r << (*i).g << (*i).b;
    }
  }
}

void printPnm(const Frame<RgbPixel>& f, ostream& os, bool binary) {
  printPpm(f, os, binary);
}

void printPnm(const Frame<PixelValue>& f, ostream& os, bool binary) {
  printPgm(f, os, binary);
}

void printPnm(const FrameWrapper& img, std::ostream& out, bool binary) {
  if (img.color) {
    printPpm(*img.colorFrame, out, binary);
  } else {
    printPgm(*img.greyFrame, out, binary);
  }
}

void writePnm(const FrameWrapper& img, string name) {
  fstream ofile(name.c_str(), fstream::out);
  printPnm(img, ofile);
  ofile.close();
}

FrameWrapper* readPnm(string name) {
  fstream ifile(name.c_str(), fstream::in);
  FrameWrapper* inputImage = loadPnm(ifile);
  ifile.close();
  return inputImage;
}
