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

#include "point.h"
#include "frame.h"

using namespace std;

// faster than list
typedef deque<Point*> Path;

// Best parent is an improvement in speed
static const bool best_parent = true;

static void addActive(FlowState& state, Point* p) {
  p->active = true;
  state.A.push(p);
}

static Point* getActive(FlowState& state) {
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
  state.O.push(p);
}

static Point* getOrphan(FlowState& state) {
  Point* result = state.O.top();
  state.O.pop();
  return result;
}

static size_t getOff(const FlowState& state, size_t x, size_t y) {
  return y * state.frame.w + x;
}

/* if into return nodes p flows into, else return nodes that flow into p
   when into=true, first link is the one that is limited*/
template<bool into, FlowDirection direction>
static void getNeighbors(Point& p, FlowState& state,
                                size_t x = 0, size_t y = 0) {
  Point::NeighborSet& result = (into?p.to:p.from);
  if (&p != &state.s && &p != &state.t) {
    result.reserve(4);
    if (direction == FLOW_LEFT_RIGHT) {
      if (x < state.frame.w-1) {
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
      if (y < state.frame.h-1) {
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
        y < state.frame.h-1 && x > 0) {
      size_t o = getOff(state, x-1, y+1);
      result.push_back(&state.points[o]);
    }
    if (!into && y < state.frame.h-1 && x < state.frame.w-1) {
      size_t o = getOff(state, x+1, y+1);
      result.push_back(&state.points[o]);
    }
    if (((direction == FLOW_LEFT_RIGHT && !into) ||
         (direction == FLOW_TOP_BOTTOM && into)) &&
        y > 0 && x < state.frame.w - 1) {
      size_t o = getOff(state, x+1, y-1);
      result.push_back(&state.points[o]);
    }
  } else if (&p == &state.s && into) {
    if (direction == FLOW_LEFT_RIGHT) {
      result.reserve(state.frame.h);
      for(size_t i = 0; i < state.frame.h; i++) {
        size_t o = getOff(state, 0, i);
        result.push_back(&state.points[o]);
      }
    } else {
      result.reserve(state.frame.w);
      for(size_t i = 0; i < state.frame.w; i++) {
        size_t o = getOff(state, i, 0);
        result.push_back(&state.points[o]);
      }
    }
  } else if (&p == &state.t && !into) {
    if (direction == FLOW_LEFT_RIGHT) {
      result.reserve(state.frame.h);
      for(size_t i = 0; i < state.frame.h; i++) {
        size_t o = getOff(state, state.frame.w-1, i);
        result.push_back(&state.points[o]);
      }
    } else {
      result.reserve(state.frame.w);
      for(size_t i = 0; i < state.frame.w; i++) {
        size_t o = getOff(state, i, state.frame.h-1);
        result.push_back(&state.points[o]);
      }
    }
  }
}

template <Point::Tree T>
static Point::EnergyType tree_cap(const Point& from, const Point& to) {
  if (T == Point::TREE_S && from.next == &to) {
    return from.capacity - from.flow;
  } else if (T == Point::TREE_T && to.next == &from) {
    return to.capacity - to.flow;
  } else {
    return 1;
  }
}

static bool is_closer(const Point& p, const Point& q) {
  return q.time <= p.time && q.dist > p.dist;
}

static void setDists(Point* p, FlowState::DistType i,
                     FlowState::TimeType time) {
  if (p->time == time) return;
  p->dist = i;
  p->time = time;
  if (p->parent != NULL) {
    return setDists(p->parent, i-1, time);
  }
}

static FlowState::DistType walkOrigin(const Point* p, FlowState::DistType i,
                                      FlowState::TimeType time) {
  if (p->time == time) {
    return p->dist + i;
  } else if (p->parent == NULL) {
    return (FlowState::DistType) ~0;
  } else {
    return walkOrigin(p->parent, i+1, time);
  }
}

static FlowState::DistType getOrigin(const FlowState& state,
                                     Point* p) {
  FlowState::DistType dist = walkOrigin(p, 0, state.time);
  if (dist != (FlowState::DistType)~0) {
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
      FlowState::DistType t = getOrigin(state, &x);
      if (t != (FlowState::DistType) ~0 && t <= dist) {
        parent = &x;
        dist = x.dist + 1;
        if (!best_parent)
          break;
      }
    }
  }
  if (parent != NULL) {
    p.parent = parent;
    p.dist = dist;
    p.time = state.time;
    return;
  } else {
    // invalidate children
    for (Point::NeighborSet::iterator i = children.begin();
         i != children.end(); ++i) {
      Point& x = **i;
      if (x.parent == &p) {
        x.parent = NULL;
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
    x.flow += bottleneck;
    if (i == P.begin() || j == P.end()) {
      x.flow += bottleneck;
    } else if (x.next == &y) {
      x.flow += bottleneck;
      if (x.flow == x.capacity && x.tree == y.tree) {
        if (x.tree == Point::TREE_S) {
          y.parent = NULL;
          addOrphan(state, &y);
        } else { // implied x.tree == Point::TREE_T
          x.parent = NULL;
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
    if (!tree_cap<T>(p, x)) {
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
      if (is_closer(p, x)) {
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
  if (state.A.empty()) return NULL;

  Point& p = *getActive(state);
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
  const Frame<unsigned char>& frame = state.frame;
  for(size_t y = 0; y < frame.h; y++) {
    for(size_t x = 0; x < frame.w; x++) {
      size_t o = getOff(state,x,y);
      Point& p = state.points[o];
      p.capacity = frame.values[o]+1;
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

FlowState* getBestFlow(const Frame<unsigned char>& frame,
                       FlowDirection direction) {
  FlowState* state = new FlowState(frame);

  state->direction = direction;

  state->s.tree = Point::TREE_S;
  state->t.tree = Point::TREE_T;

  state->s.dist = state->t.dist = 0;

  state->time = state->s.time = state->t.time = 1;

  state->points.resize(frame.h * frame.w);

  if (direction == FLOW_LEFT_RIGHT) {
    buildGraph<FLOW_LEFT_RIGHT>(*state);
  } else {
    buildGraph<FLOW_TOP_BOTTOM>(*state);
  }

  addActive(*state, &state->s);
  addActive(*state, &state->t);

  while (true) {
    Path* P = grow(*state);
    if (P == NULL) {
      return state;
    }

    state->time += 1;
    // hopefully this should never happen, but if it does...
    if (state->time == 0) {
      for (FlowState::PointsSet::iterator i = state->points.begin();
           i != state->points.end(); ++i) {
        i->time = 0;
        i->dist = 0;
      }
      state->time += 1;
    }
    state->s.time = state->time;
    state->t.time = state->time;

    augment(*state, *P);
    delete P;
    adopt(*state);
  }
}
