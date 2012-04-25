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

inline static unsigned short tree_cap(const Point& from, const Point& to) {
  if (from.next == &to) {
    return from.capacity - from.flow;
  } else {
    return 1;
  }
}

inline static Point* getOrigin(Point* p) {
  return p->origin;
}

inline static void setOrigin(Point& p, Point* origin) {
  queue<Point*, deque<Point*> > pending;
  pending.push(&p);
  while (!pending.empty()) {
    Point& x = *pending.front();
    pending.pop();
    x.origin = origin;
    if (x.tree == Point::TREE_S) {
      for (Point::NeighborSet::iterator i=x.to.begin();
           i != x.to.end(); ++i) {
        if ((*i)->parent == &x) pending.push(*i);
      }
    } else {
      for (Point::NeighborSet::iterator i = x.from.begin();
           i != x.from.end(); ++i) {
       if ((*i)->parent == &x) pending.push(*i);
      }
    }
  }
}

static void adopt(FlowState& state) {
  while (!state.O.empty()) {
    Point& p = *getOrphan(state);

    if (p.tree == Point::TREE_S) {
      // look for a parent that flows into p
      for(Point::NeighborSet::iterator i = p.from.begin();
          i != p.from.end(); ++i) {
        Point &x = **i;
        if (x.tree == Point::TREE_S && tree_cap(x, p) &&
            getOrigin(&x) == &state.s) {
          p.parent = &x;
          p.tree = Point::TREE_S;
          setOrigin(p, &state.s);
          return adopt(state);
        }
      }
      // invalidate children
      for (Point::NeighborSet::iterator i = p.to.begin();
           i != p.to.end(); ++i) {
        Point& x = **i;
        if (x.parent == &p) {
          x.parent = NULL;
          addOrphan(state, &x);
        }
      }
      // mark potential parents as active
      for (Point::NeighborSet::iterator i = p.from.begin();
           i != p.from.end(); ++i) {
        Point& x = **i;
        if (x.tree == Point::TREE_S && tree_cap(x, p)) {
          addActive(state, &x);
        }
      }
    } else {
      // look for a parent that p flows into
      for(Point::NeighborSet::iterator i = p.to.begin();
          i != p.to.end(); ++i) {
        Point &x = **i;
        if (x.tree == Point::TREE_T && tree_cap(p, x) &&
            getOrigin(&x) == &state.t) {
          p.parent = &x;
          p.tree = Point::TREE_T;
          setOrigin(p, &state.t);
          return adopt(state);
        }
      }
      // invalidate children
      for (Point::NeighborSet::iterator i = p.from.begin();
           i != p.from.end(); ++i) {
        Point& x = **i;
        if (x.parent == &p) {
          x.parent = NULL;
          addOrphan(state, &x);
        }
      }
      // mark potential parents as active
      for (Point::NeighborSet::iterator i = p.to.begin();
           i != p.to.end(); ++i) {
        Point& x = **i;
        if (x.tree == Point::TREE_T && tree_cap(p, x)) {
          addActive(state, &x);
        }
      }
    }
    p.tree = Point::TREE_NONE;
    removeActive(state, &p);
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
          setOrigin(x, NULL);
          addOrphan(state, &x);
        }
      }
    }
  }
}

static Path* getPath(Point& a, Point& b) {
  Path* result = new Path;
  Path& path = *result;
  for(Point* x = &a; x != NULL; x = x->parent) {
    path.push_front(x);
  }
  for(Point* x = &b; x != NULL; x = x->parent) {
    path.push_back(x);
  }
  return result;
}

static Path* grow(FlowState& state) {
  while (!state.A.empty()) {
    Point& p = *getActive(state);
    // manually unswitched loop
    if (p.tree == Point::TREE_S) {
      for (Point::NeighborSet::iterator i = p.to.begin();
           i != p.to.end(); ++i) {
        Point& x = **i;
        if (x.tree == Point::TREE_S || !tree_cap(p, x)) {
          continue;
        } else if (x.tree != Point::TREE_NONE) { // (*i)->tree != p->tree
          return getPath(p, x);
        } else {
          x.parent = &p;
          x.tree = Point::TREE_S;
          addActive(state, &x);
        }
      }
    } else {
      for (Point::NeighborSet::iterator i = p.from.begin();
           i != p.from.end(); ++i) {
        Point& x = **i;
        if (x.tree == Point::TREE_T || !tree_cap(x, p)) {
          continue;
        } else if (x.tree != Point::TREE_NONE) { // (*i)->tree != p->tree
          return getPath(x, p);
        } else {
          x.parent = &p;
          x.tree = Point::TREE_T;
          addActive(state, &x);
        }
      }
    }
  }
  return NULL;
}

FlowState* getBestFlow(const Frame<unsigned char>& frame,
                       FlowDirection direction) {
  FlowState* state = new FlowState(frame);

  state->direction = direction;

  state->s.tree = Point::TREE_S;
  state->t.tree = Point::TREE_T;

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
    augment(*state, *P);
    delete P;
    adopt(*state);
  }
}

