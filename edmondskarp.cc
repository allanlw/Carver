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
#include "edmondskarp.h"

#include "const.h"

using namespace std;

// faster than list
typedef deque<Point*> Path;

static void addActive(EdmondsKarpFlowState& state, Point* p) {
  p->active = true;
  state.A.push(p);
}

static Point* getActive(EdmondsKarpFlowState& state) {
  if (state.A.empty()) return NULL;
  Point* result = state.A.front();
  state.A.pop();
  if (result->active) {
    return result; // don't need to set result->active to false
  } else {
    return getActive(state);
  }
}

static void removeActive(EdmondsKarpFlowState& state, Point* p) {
  p->active = false;
}

static void addOrphan(EdmondsKarpFlowState& state, Point* p) {
  p->parent = NULL;
  state.O.push(p);
}

static Point* getOrphan(EdmondsKarpFlowState& state) {
  Point* result = state.O.top();
  state.O.pop();
  return result;
}

/* Returns > 0 if from is a valid parent of to */
template <Point::Tree T>
static FlowState::EnergyType tree_cap(const Point& from, const Point& to) {
  if (T == Point::TREE_S && from.next == &to) {
    return from.capacity - from.flow;
  } else if (T == Point::TREE_T && to.next == &from) {
    return to.capacity - to.flow;
  } else {
    return (FlowState::EnergyType)1;
  }
}

static bool is_closer(const Point& p, const Point& q) {
  return q.time <= p.time && q.dist > p.dist;
}

static void setDists(Point& p, EdmondsKarpFlowState::DistType i,
                     EdmondsKarpFlowState::TimeType time) {
  if (p.time == time) return;
  p.dist = i;
  p.time = time;
  if (p.parent != NULL) {
    return setDists(*p.parent, i-1, time);
  }
}

static EdmondsKarpFlowState::DistType walkOrigin(const Point& p, EdmondsKarpFlowState::DistType i,
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
                                     Point& p) {
  EdmondsKarpFlowState::DistType dist = walkOrigin(p, 0, state.time);
  if (dist != (EdmondsKarpFlowState::DistType)~0 && EDMONDS_KARP_USE_HEURISTIC) {
    setDists(p, dist, state.time);
  }
  return dist;
}

template <Point::Tree T>
static void do_adoption(EdmondsKarpFlowState& state, Point& p) {
  Point::NeighborSet& parents = ((T==Point::TREE_S)?p.from:p.to);
  Point::NeighborSet& children = ((T==Point::TREE_S)?p.to:p.from);

  Point* parent = NULL;
  EdmondsKarpFlowState::DistType dist = ~0;

  // look for a parent that flows into p
  for(Point::NeighborSet::iterator i = parents.begin();
      i != parents.end(); ++i) {
    Point &x = **i;
    if (x.tree == T && tree_cap<T>(x, p)) {
      EdmondsKarpFlowState::DistType t = getOrigin(state, x);
      if (t != (EdmondsKarpFlowState::DistType)~0 && t < dist) {
        parent = &x;
        dist = t;
        if (!EDMONDS_KARP_BEST_PARENT)
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

static void adopt(EdmondsKarpFlowState& state) {
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

static void augment(EdmondsKarpFlowState& state, Path& P) {
  // use two iterators to support forward iterators (i.e. lists)
  FlowState::EnergyType bottleneck = (FlowState::EnergyType)~0;
  for(Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    Point& x = **i;
    if (x.next == *j) {
      FlowState::EnergyType diff = x.capacity - x.flow;
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
static Path* do_grow(EdmondsKarpFlowState& state, Point& p) {
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
      if (EDMONDS_KARP_REASSIGN_PARENTS && EDMONDS_KARP_USE_HEURISTIC && is_closer(p, x)) {
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

FlowState::EnergyType EdmondsKarpFlowState::calcMaxFlow(FlowDirection direction) {
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
