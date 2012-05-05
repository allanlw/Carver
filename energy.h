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
#ifndef _ENERGY_H
#define _ENERGY_H

#include <deque>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <stack>

#include "frame.h"
#include "diff.h"
#include "const.h"

typedef unsigned short _FlowStateEnergyType;
typedef unsigned short _FlowStateDistType;
typedef unsigned short _FlowStateTimeType;

#include "point.h"

enum FlowDirection {
  FLOW_LEFT_RIGHT,
  FLOW_TOP_BOTTOM
};

class FlowState {
public:
  // random access required, vector/deque approx same speed.
  typedef std::vector<Point> PointsSet;

  typedef _FlowStateEnergyType EnergyType;
  typedef _FlowStateDistType DistType;
  typedef _FlowStateTimeType TimeType;

  PointsSet points;

  Point s;
  Point t;

  FlowDirection direction;
  Frame<PixelValue>* energy;
protected:
  FlowState(FrameWrapper& frame) :
    energy(getDifferential(frame)) { }
public:
  virtual EnergyType calcMaxFlow(FlowDirection direction) = 0;

  virtual FrameWrapper* cutFrame(const FrameWrapper& subject,
                                 FrameWrapper* cut);

  virtual ~FlowState() {
    delete energy;
  }
};

FlowState* getNewFlowState(FrameWrapper& frame);

inline size_t getOff(const FlowState& state, size_t x, size_t y) {
  return y * state.energy->w + x;
}

/* if into return nodes p flows into, else return nodes that flow into p
   when into=true, first link is the one that is limited*/
template<bool into, FlowDirection direction>
void populateNeighbors(Point& p, FlowState& state,
                       size_t x = 0, size_t y = 0) {
  Point::NeighborSet& result = (into?p.to:p.from);
  size_t w = state.energy->w;
  size_t h = state.energy->h;
  if (&p != &state.s && &p != &state.t) {
    result.reserve(4);
    if (direction == FLOW_LEFT_RIGHT) {
      if (x < w-1) {
        size_t o = getOff(state, x+1, y);
        result.push_back(&state.points[o]);
      } else if (into){
        result.push_back(&state.t);
      }
      if (x > 0) {
        size_t o = getOff(state, x-1, y);
        result.push_back(&state.points[o]);
      } else if (!into) {
        result.push_back(&state.s);
      }
    } else {
      if (y < h-1) {
        size_t o = getOff(state, x, y+1);
        result.push_back(&state.points[o]);
      } else if (into) {
        result.push_back(&state.t);
      }
      if (y > 0) {
        size_t o = getOff(state, x, y-1);
        result.push_back(&state.points[o]);
      } else if (!into) {
        result.push_back(&state.s);
      }
    }
    if (into && y > 0 && x > 0) {
      size_t o = getOff(state, x-1, y-1);
      result.push_back(&state.points[o]);
    }
    if (((direction == FLOW_LEFT_RIGHT && into) ||
         (direction == FLOW_TOP_BOTTOM && !into)) &&
        y < h-1 && x > 0) {
      size_t o = getOff(state, x-1, y+1);
      result.push_back(&state.points[o]);
    }
    if (!into && y < h-1 && x < w-1) {
      size_t o = getOff(state, x+1, y+1);
      result.push_back(&state.points[o]);
    }
    if (((direction == FLOW_LEFT_RIGHT && !into) ||
         (direction == FLOW_TOP_BOTTOM && into)) &&
        y > 0 && x < w - 1) {
      size_t o = getOff(state, x+1, y-1);
      result.push_back(&state.points[o]);
    }
  } else if (&p == &state.s && into) {
    if (direction == FLOW_LEFT_RIGHT) {
      result.reserve(h);
      for(size_t i = 0; i < h; i++) {
        size_t o = getOff(state, 0, i);
        result.push_back(&state.points[o]);
      }
    } else {
      result.reserve(w);
      for(size_t i = 0; i < w; i++) {
        size_t o = getOff(state, i, 0);
        result.push_back(&state.points[o]);
      }
    }
  } else if (&p == &state.t && !into) {
    if (direction == FLOW_LEFT_RIGHT) {
      result.reserve(h);
      for(size_t i = 0; i < h; i++) {
        size_t o = getOff(state, w-1, i);
        result.push_back(&state.points[o]);
      }
    } else {
      result.reserve(w);
      for(size_t i = 0; i < w; i++) {
        size_t o = getOff(state, i, h-1);
        result.push_back(&state.points[o]);
      }
    }
  }
}

template<FlowDirection direction>
void buildGraph(FlowState& state) {
  const Frame<PixelValue>& frame = *state.energy;
  for(size_t y = 0; y < frame.h; y++) {
    for(size_t x = 0; x < frame.w; x++) {
      size_t o = getOff(state, x, y);
      Point& p = state.points[o];
      p.capacity = frame.values[o] + 1;
      p.flow = 0;
      populateNeighbors<true, direction>(p, state, x, y);
      if (p.to.front() != &state.t)
        p.next = p.to.front();
      populateNeighbors<false, direction>(p, state, x, y);
    }
  }

  populateNeighbors<true, direction>(state.s, state);
  populateNeighbors<false, direction>(state.s, state);
  populateNeighbors<true, direction>(state.t, state);
  populateNeighbors<false, direction>(state.t, state);
}

#endif
