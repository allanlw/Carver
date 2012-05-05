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
#include "edmondskarpenergy.h"

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

#include "edmondskarppoint.h"

using namespace std;

// faster than list
typedef deque<EdmondsKarpPoint*> Path;

// Best parent is an improvement in speed
static const bool best_parent = true;
// Use the heuristic algorithm
static const bool use_heuristic = true;
// Reassign parents (requires heuristic)
static const bool reassign_parents = true;

static void addActive(EdmondsKarpFlowState& state, EdmondsKarpPoint* p) {
  p->active = true;
  state.A.push(p);
}

static EdmondsKarpPoint* getActive(EdmondsKarpFlowState& state) {
  if (state.A.empty()) return NULL;
  EdmondsKarpPoint* result = state.A.front();
  state.A.pop();
  if (result->active) {
    return result; // don't need to set result->active to false
  } else {
    return getActive(state);
  }
}

static void removeActive(EdmondsKarpFlowState& state, EdmondsKarpPoint* p) {
  p->active = false;
}

static void addOrphan(EdmondsKarpFlowState& state, EdmondsKarpPoint* p) {
  p->parent = NULL;
  state.O.push(p);
}

static EdmondsKarpPoint* getOrphan(EdmondsKarpFlowState& state) {
  EdmondsKarpPoint* result = state.O.top();
  state.O.pop();
  return result;
}

static size_t getOff(const EdmondsKarpFlowState& state, size_t x, size_t y) {
  return y * state.energy->w + x;
}

/* if into return nodes p flows into, else return nodes that flow into p
   when into=true, first link is the one that is limited*/
template<bool into, FlowDirection direction>
static void getNeighbors(EdmondsKarpPoint& p, EdmondsKarpFlowState& state,
                         size_t x = 0, size_t y = 0) {
  EdmondsKarpPoint::NeighborSet& result = (into?p.to:p.from);
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
template <EdmondsKarpPoint::Tree T>
static FlowState::EnergyType tree_cap(const EdmondsKarpPoint& from, const EdmondsKarpPoint& to) {
  if (T == EdmondsKarpPoint::TREE_S && from.next == &to) {
    return from.capacity - from.flow;
  } else if (T == EdmondsKarpPoint::TREE_T && to.next == &from) {
    return to.capacity - to.flow;
  } else {
    return (FlowState::EnergyType)1;
  }
}

static bool is_closer(const EdmondsKarpPoint& p, const EdmondsKarpPoint& q) {
  return q.time <= p.time && q.dist > p.dist;
}

static void setDists(EdmondsKarpPoint& p, EdmondsKarpFlowState::DistType i,
                     EdmondsKarpFlowState::TimeType time) {
  if (p.time == time) return;
  p.dist = i;
  p.time = time;
  if (p.parent != NULL) {
    return setDists(*p.parent, i-1, time);
  }
}

static EdmondsKarpFlowState::DistType walkOrigin(const EdmondsKarpPoint& p, EdmondsKarpFlowState::DistType i,
                                      EdmondsKarpFlowState::TimeType time) {
  if (p.time == time) {
    return p.dist + i;
  } else if (p.parent == NULL) {
    return (EdmondsKarpFlowState::DistType) ~0;
  } else {
    return walkOrigin(*p.parent, i+1, time);
  }
}

/* Returns the distance from a terminal, ~0 if not connected */
static EdmondsKarpFlowState::DistType getOrigin(const EdmondsKarpFlowState& state,
                                     EdmondsKarpPoint& p) {
  EdmondsKarpFlowState::DistType dist = walkOrigin(p, 0, state.time);
  if (dist != (EdmondsKarpFlowState::DistType)~0 && use_heuristic) {
    setDists(p, dist, state.time);
  }
  return dist;
}

template <EdmondsKarpPoint::Tree T>
static void do_adoption(EdmondsKarpFlowState& state, EdmondsKarpPoint& p) {
  EdmondsKarpPoint::NeighborSet& parents = ((T==EdmondsKarpPoint::TREE_S)?p.from:p.to);
  EdmondsKarpPoint::NeighborSet& children = ((T==EdmondsKarpPoint::TREE_S)?p.to:p.from);

  EdmondsKarpPoint* parent = NULL;
  EdmondsKarpFlowState::DistType dist = ~0;

  // look for a parent that flows into p
  for(EdmondsKarpPoint::NeighborSet::iterator i = parents.begin();
      i != parents.end(); ++i) {
    EdmondsKarpPoint &x = **i;
    if (x.tree == T && tree_cap<T>(x, p)) {
      EdmondsKarpFlowState::DistType t = getOrigin(state, x);
      if (t != (EdmondsKarpFlowState::DistType)~0 && t < dist) {
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
    for (EdmondsKarpPoint::NeighborSet::iterator i = children.begin();
         i != children.end(); ++i) {
      EdmondsKarpPoint& x = **i;
      if (x.parent == &p) {
        addOrphan(state, &x);
      }
    }
    // mark potential parents as active
    for (EdmondsKarpPoint::NeighborSet::iterator i = parents.begin();
         i != parents.end(); ++i) {
      EdmondsKarpPoint& x = **i;
      if (x.tree == T && tree_cap<T>(x, p)) {
        addActive(state, &x);
      }
    }
    p.tree = EdmondsKarpPoint::TREE_NONE;
    removeActive(state, &p);
  }
}

static void adopt(EdmondsKarpFlowState& state) {
  if (!state.O.empty()) {
    EdmondsKarpPoint& p = *getOrphan(state);

    if (p.tree == EdmondsKarpPoint::TREE_S) {
      do_adoption<EdmondsKarpPoint::TREE_S>(state, p);
    } else {
      do_adoption<EdmondsKarpPoint::TREE_T>(state, p);
    }
    return adopt(state);
  }
}

static void augment(EdmondsKarpFlowState& state, Path& P) {
  // use two iterators to support forward iterators (i.e. lists)
  FlowState::EnergyType bottleneck = (FlowState::EnergyType)~0;
  for(Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    EdmondsKarpPoint& x = **i;
    if (x.next == *j) {
      FlowState::EnergyType diff = x.capacity - x.flow;
      if (diff < bottleneck) {
        bottleneck = diff;
      }
    }
  }
  for (Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    EdmondsKarpPoint& x = **i;
    EdmondsKarpPoint& y = **j;
    if (i == P.begin()) {
      x.flow += bottleneck;
    } else if (x.next == &y) {
      x.flow += bottleneck;
      if(x.flow >= x.capacity && x.tree == y.tree) {
        if (x.tree == EdmondsKarpPoint::TREE_S) {
          addOrphan(state, &y);
        } else { // implied x.tree == EdmondsKarpPoint::TREE_T
          addOrphan(state, &x);
        }
      }
    }
  }
}

template<EdmondsKarpPoint::Tree T>
static Path* getPath(EdmondsKarpPoint& a, EdmondsKarpPoint& b) {
  Path* result = new Path;
  Path& path = *result;
  for(EdmondsKarpPoint* x = (T==EdmondsKarpPoint::TREE_S)?(&a):(&b); x != NULL; x = x->parent) {
    path.push_front(x);
  }
  for(EdmondsKarpPoint* x = (T==EdmondsKarpPoint::TREE_S)?(&b):(&a); x != NULL; x = x->parent) {
    path.push_back(x);
  }
  return result;
}

template<EdmondsKarpPoint::Tree T>
static Path* do_grow(EdmondsKarpFlowState& state, EdmondsKarpPoint& p) {
  EdmondsKarpPoint::NeighborSet& children = (T==EdmondsKarpPoint::TREE_S)?p.to:p.from;
  for (EdmondsKarpPoint::NeighborSet::iterator i = children.begin();
       i != children.end(); ++i) {
    EdmondsKarpPoint& x = **i;
    if (tree_cap<T>(p, x) == 0) {
      continue;
    }
    switch(x.tree) {
    case EdmondsKarpPoint::TREE_NONE:
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

static Path* grow(EdmondsKarpFlowState& state) {
  EdmondsKarpPoint* t = getActive(state);
  if (t == NULL) return NULL;
  EdmondsKarpPoint& p = *t;

  Path* result;
  if (p.tree==EdmondsKarpPoint::TREE_S) {
    result = do_grow<EdmondsKarpPoint::TREE_S>(state, p);
  } else {
    result = do_grow<EdmondsKarpPoint::TREE_T>(state, p);
  }
  if (result != NULL)
    return result;
  else
    return grow(state);
}

template<FlowDirection direction>
static void buildGraph(EdmondsKarpFlowState& state) {
  const Frame<PixelValue>& frame = *state.energy;
  for(size_t y = 0; y < frame.h; y++) {
    for(size_t x = 0; x < frame.w; x++) {
      size_t o = getOff(state, x, y);
      EdmondsKarpPoint& p = state.points[o];
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

FlowState::EnergyType EdmondsKarpFlowState::calcMaxFlow(FlowDirection direction) {
  this->direction = direction;

  A = ActiveSet();
  O = OrphanSet();

  s = EdmondsKarpPoint();
  t = EdmondsKarpPoint();

  s.tree = EdmondsKarpPoint::TREE_S;
  t.tree = EdmondsKarpPoint::TREE_T;

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
      return s.flow;
    }

    time += 1;
    // hopefully this should never happen, but if it does...
    if (time == 0) {
      for (EdmondsKarpFlowState::PointsSet::iterator i = points.begin();
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

FrameWrapper* EdmondsKarpFlowState::cutFrame(const FrameWrapper& subject,
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
    if (points[i].tree == EdmondsKarpPoint::TREE_T ||
        points[i].tree == EdmondsKarpPoint::TREE_NONE) {
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
