#ifndef _ENERGY_H
#define _ENERGY_H

#include <deque>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <stack>

#include "point.h"
#include "frame.h"

enum FlowDirection {
  FLOW_LEFT_RIGHT,
  FLOW_TOP_BOTTOM
};

struct FlowState {
public:
  // random access required, vector/deque approx same speed.
  typedef std::vector<Point> PointsSet;
  // operations: add, remove something deque clearly faster
  // queue much faster than stack (algorithmically)
  typedef std::queue<Point*, std::deque<Point*> > ActiveSet;
  // operators: add, remove something deque clearly faster than list
  // potential (small) speedup from using a vector with a large reserved size.
  typedef std::stack<Point*, std::deque<Point*> > OrphanSet;

  std::size_t time;

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
    T nil;
    memset(&nil, 0x00, sizeof(T));
    for (std::size_t x = 0; x < cut->w * cut->h; x++) {
      cut->values[x] = nil;
    }
  }

  T one;
  memset(&one, 0xff, sizeof(T));

  result->values.resize(result->w*result->h);
  for(size_t i = 0; i < state.points.size(); i++) {
    std::size_t x = i % subject.w, y = i / subject.w;
    std::size_t tox = -1, toy = -1;
    if (state.points[i].tree == Point::TREE_T) {
      if (state.direction == FLOW_LEFT_RIGHT && x > 0) {
        tox = x - 1;
        toy = y;
      } else if (state.direction == FLOW_TOP_BOTTOM && y > 0) {
        tox = x;
        toy = y - 1;
      }
    } else {
      tox = x;
      toy = y;
    }
    result->values[tox + toy*result->w] = subject.values[x+y*subject.w];
    if (cut != NULL) {
      cut->values[tox + toy*cut->w] ^= one;
    }
  }
  return result;
}

#endif
