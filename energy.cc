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
#include "energy.h"

#include <vector>
#include <set>
#include <map>
#include <list>
#include <cstddef>
#include <utility>
#include <limits>
#include <algorithm>
#include <stack>
#include <iostream>

#include "point.h"
#include "frame.h"
#include "diff.h"

using namespace std;

// faster than list
typedef deque<Point*> Path;

// Best parent is an improvement in speed
static const bool best_parent = true;
// Use the heuristic algorithm
static const bool use_heuristic = true;
// Reassign parents (requires heuristic)
static const bool reassign_parents = true;

static void addActive(FlowState& state, Point* p) {
  p->active = true;
  state.A.push(p);
}

static Point* getActive(FlowState& state) {
  if (state.A.empty()) return NULL;
  Point* result = state.A.front();
  state.A.pop();
  if (result->active) {
    return result; // don't need to set result->active to false
  } else {
    return getActive(state);
  }
}

static void removeActive(FlowState& state, Point* p) {
  p->active = false;
}

static void addOrphan(FlowState& state, Point* p) {
  p->parent = NULL;
  state.O.push(p);
}

static Point* getOrphan(FlowState& state) {
  Point* result = state.O.top();
  state.O.pop();
  return result;
}

static size_t getOff(const FlowState& state, size_t x, size_t y) {
  return y * state.energy->w + x;
}

/* if into return nodes p flows into, else return nodes that flow into p
   when into=true, first link is the one that is limited*/
template<bool into, FlowDirection direction>
static void getNeighbors(Point& p, FlowState& state,
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

/* Returns > 0 if from is a valid parent of to */
template <Point::Tree T>
static Point::EnergyType tree_cap(const Point& from, const Point& to) {
  if (T == Point::TREE_S && from.next == &to) {
    return from.capacity - from.flow;
  } else if (T == Point::TREE_T && to.next == &from) {
    return to.capacity - to.flow;
  } else {
    return (Point::EnergyType)1;
  }
}

static bool is_closer(const Point& p, const Point& q) {
  return q.time <= p.time && q.dist > p.dist;
}

static void setDists(Point& p, FlowState::DistType i,
                     FlowState::TimeType time) {
  if (p.time == time) return;
  p.dist = i;
  p.time = time;
  if (p.parent != NULL) {
    return setDists(*p.parent, i-1, time);
  }
}

static FlowState::DistType walkOrigin(const Point& p, FlowState::DistType i,
                                      FlowState::TimeType time) {
  if (p.time == time) {
    return p.dist + i;
  } else if (p.parent == NULL) {
    return (FlowState::DistType) ~0;
  } else {
    return walkOrigin(*p.parent, i+1, time);
  }
}

/* Returns the distance from a terminal, ~0 if not connected */
static FlowState::DistType getOrigin(const FlowState& state,
                                     Point& p) {
  FlowState::DistType dist = walkOrigin(p, 0, state.time);
  if (dist != (FlowState::DistType)~0 && use_heuristic) {
    setDists(p, dist, state.time);
  }
  return dist;
}

template <Point::Tree T>
static void do_adoption(FlowState& state, Point& p) {
  Point::NeighborSet& parents = ((T==Point::TREE_S)?p.from:p.to);
  Point::NeighborSet& children = ((T==Point::TREE_S)?p.to:p.from);

  Point* parent = NULL;
  FlowState::DistType dist = ~0;

  // look for a parent that flows into p
  for(Point::NeighborSet::iterator i = parents.begin();
      i != parents.end(); ++i) {
    Point &x = **i;
    if (x.tree == T && tree_cap<T>(x, p)) {
      FlowState::DistType t = getOrigin(state, x);
      if (t != (FlowState::DistType)~0 && t < dist) {
        parent = &x;
        dist = t;
        if (!best_parent)
          break;
      }
    }
  }
  if (parent != NULL) {
    p.parent = parent;
    p.dist = dist + 1;
    p.time = state.time;
  } else {
    // invalidate children
    for (Point::NeighborSet::iterator i = children.begin();
         i != children.end(); ++i) {
      Point& x = **i;
      if (x.parent == &p) {
        addOrphan(state, &x);
      }
    }
    // mark potential parents as active
    for (Point::NeighborSet::iterator i = parents.begin();
         i != parents.end(); ++i) {
      Point& x = **i;
      if (x.tree == T && tree_cap<T>(x, p)) {
        addActive(state, &x);
      }
    }
    p.tree = Point::TREE_NONE;
    removeActive(state, &p);
  }
}

static void adopt(FlowState& state) {
  if (!state.O.empty()) {
    Point& p = *getOrphan(state);

    if (p.tree == Point::TREE_S) {
      do_adoption<Point::TREE_S>(state, p);
    } else {
      do_adoption<Point::TREE_T>(state, p);
    }
    return adopt(state);
  }
}

static void augment(FlowState& state, Path& P) {
  // use two iterators to support forward iterators (i.e. lists)
  Point::EnergyType bottleneck = (Point::EnergyType)~0;
  for(Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    Point& x = **i;
    if (x.next == *j) {
      Point::EnergyType diff = x.capacity - x.flow;
      if (diff < bottleneck) {
        bottleneck = diff;
      }
    }
  }
  for (Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    Point& x = **i;
    Point& y = **j;
    if (i == P.begin()) {
      x.flow += bottleneck;
    } else if (x.next == &y) {
      x.flow += bottleneck;
      if(x.flow >= x.capacity && x.tree == y.tree) {
        if (x.tree == Point::TREE_S) {
          addOrphan(state, &y);
        } else { // implied x.tree == Point::TREE_T
          addOrphan(state, &x);
        }
      }
    }
  }
}

template<Point::Tree T>
static Path* getPath(Point& a, Point& b) {
  Path* result = new Path;
  Path& path = *result;
  for(Point* x = (T==Point::TREE_S)?(&a):(&b); x != NULL; x = x->parent) {
    path.push_front(x);
  }
  for(Point* x = (T==Point::TREE_S)?(&b):(&a); x != NULL; x = x->parent) {
    path.push_back(x);
  }
  return result;
}

template<Point::Tree T>
static Path* do_grow(FlowState& state, Point& p) {
  Point::NeighborSet& children = (T==Point::TREE_S)?p.to:p.from;
  for (Point::NeighborSet::iterator i = children.begin();
       i != children.end(); ++i) {
    Point& x = **i;
    if (tree_cap<T>(p, x) == 0) {
      continue;
    }
    switch(x.tree) {
    case Point::TREE_NONE:
      x.parent = &p;
      x.tree = T;
      x.dist = p.dist + 1;
      x.time = p.time;
      addActive(state, &x);
      break;
    case T:
      if (reassign_parents && use_heuristic && is_closer(p, x)) {
        x.parent = &p;
        x.dist = p.dist + 1;
        x.time = p.time;
      }
      break;
    default:
      return getPath<T>(p, x);
    }
  }
  return NULL;
}

static Path* grow(FlowState& state) {
  Point* t = getActive(state);
  if (t == NULL) return NULL;
  Point& p = *t;

  Path* result;
  if (p.tree==Point::TREE_S) {
    result = do_grow<Point::TREE_S>(state, p);
  } else {
    result = do_grow<Point::TREE_T>(state, p);
  }
  if (result != NULL)
    return result;
  else
    return grow(state);
}

template<FlowDirection direction>
static void buildGraph(FlowState& state) {
  const Frame<PixelValue>& frame = *state.energy;
  for(size_t y = 0; y < frame.h; y++) {
    for(size_t x = 0; x < frame.w; x++) {
      size_t o = getOff(state, x, y);
      Point& p = state.points[o];
      p.capacity = frame.values[o] + 1;
      p.flow = 0;
      getNeighbors<true, direction>(p, state, x, y);
      if (p.to.front() != &state.t)
        p.next = p.to.front();
      getNeighbors<false, direction>(p, state, x, y);
    }
  }

  getNeighbors<true, direction>(state.s, state);
  getNeighbors<false, direction>(state.s, state);
  getNeighbors<true, direction>(state.t, state);
  getNeighbors<false, direction>(state.t, state);
}

void FlowState::calcBestFlow(FlowDirection direction) {
  this->direction = direction;

  A = ActiveSet();
  O = OrphanSet();

  s = Point();
  t = Point();

  s.tree = Point::TREE_S;
  t.tree = Point::TREE_T;

  s.dist = t.dist = 0;

  time = s.time = t.time = 1;

  points.clear();
  points.resize(energy->h * energy->w);

  if (direction == FLOW_LEFT_RIGHT) {
    buildGraph<FLOW_LEFT_RIGHT>(*this);
  } else {
    buildGraph<FLOW_TOP_BOTTOM>(*this);
  }

  addActive(*this, &s);
  addActive(*this, &t);

  while (true) {
    Path* P = grow(*this);
    if (P == NULL) {
      return;
    }

    time += 1;
    // hopefully this should never happen, but if it does...
    if (time == 0) {
      for (FlowState::PointsSet::iterator i = points.begin();
           i != points.end(); ++i) {
        i->time = 0;
        i->dist = 0;
      }
      time += 1;
    }
    s.time = t.time = time;

    augment(*this, *P);
    delete P;
    adopt(*this);
  }
}

FrameWrapper* FlowState::cutFrame(const FrameWrapper& subject,
                                  FrameWrapper* cut) {
  if ((subject.getWidth() != this->energy->w ||
       subject.getHeight() != this->energy->h) ||
      (cut != NULL && (cut->getWidth() != subject.getWidth() ||
                       cut->getHeight() != subject.getHeight()))) {
    return NULL;
  }

  FrameWrapper* result = new FrameWrapper(subject.color);
  if (direction == FLOW_LEFT_RIGHT) {
    result->setSize(subject.getWidth()-1, subject.getHeight());
  } else {
    result->setSize(subject.getWidth(), subject.getHeight()-1);
  }

  Frame<PixelValue>* newEnergy = new Frame<PixelValue>(result->getWidth(),
                                                       result->getHeight());

  if (cut != NULL) {
    zeroFrame(*cut);
  }

  for(size_t i = 0; i < points.size(); i++) {
    std::size_t x = i % subject.getWidth(), y = i / subject.getWidth();
    std::size_t tox, toy;
    if (points[i].tree == Point::TREE_T ||
        points[i].tree == Point::TREE_NONE) {
      if (direction == FLOW_LEFT_RIGHT && x > 0) {
        tox = x - 1;
        toy = y;
      } else if (direction == FLOW_TOP_BOTTOM && y > 0) {
        tox = x;
        toy = y - 1;
      } else {
        tox = x;
        toy = y;
      }
    } else {
      tox = x;
      toy = y;
    }
    if (subject.color) {
      result->colorFrame->values[tox + toy * result->getWidth()] =
        subject.colorFrame->values[x + y * subject.getWidth()];
    } else {
      result->greyFrame->values[tox + toy * result->getWidth()] =
        subject.greyFrame->values[x + y * subject.getWidth()];
    }
    newEnergy->values[tox + toy * newEnergy->w] =
      this->energy->values[x + y * this->energy->w];
    if (cut != NULL) {
      togglePixel(*cut, tox, toy);
    }
  }
  delete this->energy;
  this->energy = newEnergy;
  return result;
}
