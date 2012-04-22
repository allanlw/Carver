#include "frame.h"

#include <string>
#include <cctype>

using namespace std;

string getMagic(std::istream& is) {
  char c = is.get();
  char c2 = is.peek();
  is.unget();
  string res;
  res += c;
  res += c2;
  return res;
}

Frame<unsigned char>* loadPgm(std::istream& is) {
  Frame<unsigned char>* r = new Frame<unsigned char>();

  string line;

  deque<unsigned int> values;

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
      values.push_back(is.get());
    } else if (c == '#') {
      comment = true;
      is.get();
    } else if (c == '\n') {
      comment = false;
      is.get();
    } else if (isdigit(c)) {
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
  float max = values.front();
  values.pop_front();
  while (values.size()) {
    r->values.push_back(values.front()/max * 255);
    values.pop_front();
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
    } else if (c == '#') {
      comment = true;
      is.get();
    } else if (c == '\n') {
      comment = false;
      is.get();
    } else if (isdigit(c)) {
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
  float max = values.front();
  values.pop_front();
  while (values.size()) {
    RgbPixel p;
    p.r = values.front()/max * 255;
    values.pop_front();
    p.g = values.front()/max * 255;
    values.pop_front();
    p.b = values.front()/max * 255;
    values.pop_front();
    r->values.push_back(p);
  }

  return r;
}

void printPgm(const Frame<unsigned char>& f, ostream& os, bool binary) {
  os << (binary?"P5":"P2")<<"\n";
  os << f.w << " " << f.h << "\n";
  os << 255 << "\n";
  size_t j = 0;
  for (deque<unsigned char>::const_iterator i = f.values.begin();
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
  for (deque<RgbPixel>::const_iterator i = f.values.begin();
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

void printPnm(const Frame<unsigned char>& f, ostream& os, bool binary) {
  printPgm(f, os, binary);
}
