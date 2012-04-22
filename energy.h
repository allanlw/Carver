#ifndef _ENERGY_H
#define _ENERGY_H

#include <set>
#include <deque>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <stack>
#include <queue>

#ifndef DEBUG
#define DEBUG 0
#endif

#include "point.h"
#include "frame.h"

enum FlowDirection {
  FLOW_LEFT_RIGHT,
  FLOW_TOP_BOTTOM
};

struct FlowState {
public:
  // random access required, vector/deque approx same speed.
  typedef std::deque<Point> PointsSet;
  // operations: add el, remove el, remove something
  typedef std::deque<Point*> ActiveSet;
  // operators: add el, remove something
  typedef std::queue<Point*, std::deque<Point*> > OrphanSet;

  PointsSet points;

  ActiveSet A;
  OrphanSet O;

  Point s;
  Point t;

  FlowDirection direction;
  const Frame<unsigned char>& frame;

  FlowState(const Frame<unsigned char>& frame) : frame(frame) {}
};

FlowState* getBestFlow(const Frame<unsigned char>& frame,
                       FlowDirection direction);

template<typename T>
Frame<T>* cutFrame(FlowState& state, const Frame<T>& subject,
                   Frame<T>* cut) {
  if (subject.w != state.frame.w || subject.h != state.frame.h ||
      (cut != NULL && (cut->w != subject.w || cut->h != subject.h))) {
    return NULL;
  }

  Frame<T>* result = new Frame<T>();
  if (state.direction == FLOW_LEFT_RIGHT) {
    result->w = subject.w-1;
    result->h = subject.h;
  } else {
    result->w = subject.w;
    result->h = subject.h-1;
  }

  if (cut != NULL) {
    T* nil = new T();
    memset(nil, 0x00, sizeof(T));
    for (std::size_t x = 0; x < cut->w * cut->h; x++) {
      cut->values[x] = *nil;
    }
    delete nil;
  }

  if (DEBUG) {
    std::size_t x = 0;
    for(FlowState::PointsSet::iterator i = state.points.begin();
        i != state.points.end(); ++i) {
      std::cout << i->flow << "/" << i->capacity <<  " ";
      if (++x == state.frame.w) {
        std::cout << "\n";
        x = 0;
      }
    }
    std::cout << "\n";
  }

  T* one = new T();
  memset(one, 0xff, sizeof(T));
  std::size_t row = 0;

  result->values.resize(result->w*result->h);
  for(FlowState::PointsSet::iterator i = state.points.begin();
      i != state.points.end(); ++i) {
    std::size_t tox = -1, toy = -1;
    if (i->tree == &state.t) {
      if (DEBUG) {
        std::cout << "#";
      }
      if (state.direction == FLOW_LEFT_RIGHT && i->x > 0) {
        tox = i->x - 1;
        toy = i->y;
      } else if (state.direction == FLOW_TOP_BOTTOM && i->y > 0) {
        tox = i->x;
        toy = i->y - 1;
      }
    } else {
      if (DEBUG) {
        if (i->tree == &state.s) {
          std::cout << "O";
        } else {
          std::cout << "?";
        }
      }
      tox = i->x;
      toy = i->y;
    }
    result->values[tox + toy*result->w] = subject.values[i->x+i->y*subject.w];
    if (cut != NULL) {
      cut->values[tox + toy*cut->w] ^= *one;
    }
    if (DEBUG) {
      if (++row == state.frame.w) {
        std::cout << "\n";
        row = 0;
      }
    }
  }
  if (DEBUG) {
    std::cout << "\n";
  }
  delete one;
  return result;
}

#endif
