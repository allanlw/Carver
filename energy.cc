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

inline static void addActive(FlowState& state, Point* p) {
  p->active = true;
  state.A.push(p);
}

inline static Point* getActive(FlowState& state) {
  Point* result = state.A.front();
  state.A.pop();
  if (result->active) {
    return result; // don't need to set result->active to false
  } else {
    return getActive(state);
  }
}

inline static void removeActive(FlowState& state, Point* p) {
  p->active = false;
}

inline static void addOrphan(FlowState& state, Point* p) {
  state.O.push(p);
}

inline static Point* getOrphan(FlowState& state) {
  Point* result = state.O.top();
  state.O.pop();
  return result;
}

inline static size_t getOff(const FlowState& state, size_t x, size_t y) {
  return y * state.frame.w + x;
}

/* if into return nodes p flows into, else return nodes that flow into p
   when into=true, first link is the one that is limited*/
template<bool into>
inline static void getNeighbors(Point& p, FlowState& state,
                                size_t x = 0, size_t y = 0) {
  Point::NeighborSet& result = (into?p.to:p.from);
  if (&p != &state.s && &p != &state.t) {
    result.reserve(4);
    if (state.direction == FLOW_LEFT_RIGHT) {
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
    if (((state.direction == FLOW_LEFT_RIGHT && into) ||
         (state.direction == FLOW_TOP_BOTTOM && !into)) &&
        y < state.frame.h-1 && x > 0) {
      size_t o = getOff(state, x-1, y+1);
      result.push_back(&state.points[o]);
    }
    if (!into && y < state.frame.h-1 && x < state.frame.w-1) {
      size_t o = getOff(state, x+1, y+1);
      result.push_back(&state.points[o]);
    }
    if (((state.direction == FLOW_LEFT_RIGHT && !into) ||
         (state.direction == FLOW_TOP_BOTTOM && into)) &&
        y > 0 && x < state.frame.w - 1) {
      size_t o = getOff(state, x+1, y-1);
      result.push_back(&state.points[o]);
    }
  } else if (&p == &state.s && into) {
    if (state.direction == FLOW_LEFT_RIGHT) {
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
    if (state.direction == FLOW_LEFT_RIGHT) {
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
inline static unsigned short tree_cap(const Point& from, const Point& to) {
  if (T == Point::TREE_S && from.next == &to) {
    return from.capacity - from.flow;
  } else if (T == Point::TREE_T && to.next == &from) {
    return to.capacity - to.flow;
  } else {
    return 1;
  }
}

inline static bool is_closer(const Point& p, const Point& q) {
  return q.time <= p.time && q.dist > p.dist;
}

inline static void setDists(Point* p, size_t time, size_t i) {
  if (p->time == time) return;
  p->dist = i;
  p->time = time;
  if (p->parent) {
    return setDists(p->parent, time, i-1);
  }
}

inline static size_t walkOrigin(Point* p, size_t time, size_t i = 0) {
  if (p->time == time) {
    return p->dist + i;
  } else if (p->parent == NULL) {
    return ~0;
  } else {
    return walkOrigin(p->parent, time, i+1);
  }
}

inline static size_t getOrigin(FlowState& state, Point* p) {
  size_t dist = walkOrigin(p, state.time);
  if (dist != ~0) {
    setDists(p, state.time, dist);
  }
  return dist;
}

template <Point::Tree T>
inline static void do_adoption(FlowState& state, Point& p) {
  Point::NeighborSet& parents = ((T==Point::TREE_S)?p.from:p.to);
  Point::NeighborSet& children = ((T==Point::TREE_S)?p.to:p.from);

  Point* bestNewParent = NULL;
  // look for a parent that flows into p
  for(Point::NeighborSet::iterator i = parents.begin();
      i != parents.end(); ++i) {
    Point &x = **i;
    if (x.tree == T && tree_cap<T>(x, p)) {
      size_t t = getOrigin(state, &x);
      if (t != ~0 && (bestNewParent == NULL || t < bestNewParent->dist)) {
        bestNewParent = &x;
      }
    }
  }
  if (bestNewParent != NULL) {
    p.parent = bestNewParent;
    p.dist = bestNewParent->dist + 1;
    p.time = state.time;
    return;
  }
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
  unsigned short bottleneck = ~0;
  for(Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    Point& x = **i;
    if (x.next == *j) {
      unsigned short diff = x.capacity - x.flow;
      if (diff < bottleneck) {
        bottleneck = diff;
      }
    }
  }
  for (Path::iterator j = P.begin(), i = j++; j != P.end(); ++i, ++j) {
    Point& x = **i;
    Point& y = **j;
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
inline static Path* getPath(Point& a, Point& b) {
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
inline static Path* do_grow(FlowState& state, Point& p) {
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

FlowState* getBestFlow(const Frame<unsigned char>& frame,
                       FlowDirection direction) {
  FlowState* state = new FlowState(frame);

  state->direction = direction;

  state->s.tree = Point::TREE_S;
  state->t.tree = Point::TREE_T;

  state->s.dist = state->t.dist = 0;

  state->time = state->s.time = state->t.time = 1;

  state->points.resize(frame.h * frame.w);

  for(size_t y = 0; y < frame.h; y++) {
    for(size_t x = 0; x < frame.w; x++) {
      size_t o = getOff(*state,x,y);
      Point& p = state->points[o];
      p.capacity = frame.values[o]+1;
      getNeighbors<true>(p, *state, x, y);
      if (p.to.front() != &state->t)
        p.next = p.to.front();
      getNeighbors<false>(p, *state, x, y);
    }
  }

  getNeighbors<true>(state->s, *state);
  getNeighbors<false>(state->s, *state);
  getNeighbors<true>(state->t, *state);
  getNeighbors<false>(state->t, *state);

  addActive(*state, &state->s);
  addActive(*state, &state->t);

  while (true) {
    Path* P = grow(*state);
    if (P == NULL) {
      return state;
    }

    state->time += 1;
    state->s.time = state->time;
    state->t.time = state->time;

    augment(*state, *P);
    delete P;
    adopt(*state);
  }
}

