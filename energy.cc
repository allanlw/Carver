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

// slightly faster than list
typedef deque<Point*> Path;
// much faster than list, slightly faster than vector
typedef stack<Point*, deque<Point*> > PointStack;

inline static void addActive(FlowState& state, Point* p) {
  state.A.insert(p);
}

inline static Point* getActive(FlowState& state) {
  FlowState::ActiveSet::iterator i = state.A.begin();
  Point* result = *i;
  state.A.erase(i);
  return result;
}

inline static void removeActive(FlowState& state, Point* p) {
  state.A.erase(p);
}

inline static void addOrphan(FlowState& state, Point* p) {
  state.O.push_back(p);
}

inline static Point* getOrphan(FlowState& state) {
  FlowState::OrphanSet::iterator i = state.O.begin();
  Point* result = *i;
  state.O.erase(i);
  return result;
}

inline static size_t getOff(const FlowState& state, size_t x, size_t y) {
  return y * state.frame.w + x;
}

/* if into return nodes p flows into, else return nodes that flow into p
   when into=true, first link is the one that is limited*/
Point::NeighborSet getNeighbors(const Point& p, FlowState& state, bool into) {
  Point::NeighborSet result;
  if (&p != &state.s && &p != &state.t) {
    if (state.direction == FLOW_LEFT_RIGHT) {
      if (p.x < state.frame.w-1) {
        size_t o = getOff(state, p.x+1, p.y);
        result.push_back(&state.points[o]);
      } else if (into){
        result.push_back(&state.t);
      }
      if (p.x > 0) {
        size_t o = getOff(state, p.x-1, p.y);
        result.push_back(&state.points[o]);
      } else if (!into) {
        result.push_back(&state.s);
      }
    } else {
      if (p.y < state.frame.h-1) {
        size_t o = getOff(state, p.x, p.y+1);
        result.push_back(&state.points[o]);
      } else if (into) {
        result.push_back(&state.t);
      }
      if (p.y > 0) {
        size_t o = getOff(state, p.x, p.y-1);
        result.push_back(&state.points[o]);
      } else if (!into) {
        result.push_back(&state.s);
      }
    }
    if (into && p.y > 0 && p.x > 0) {
      size_t o = getOff(state, p.x-1, p.y-1);
      result.push_back(&state.points[o]);
    }
    if (((state.direction == FLOW_LEFT_RIGHT && into) ||
         (state.direction == FLOW_TOP_BOTTOM && !into)) &&
        p.y < state.frame.h-1 && p.x > 0) {
      size_t o = getOff(state, p.x-1, p.y+1);
      result.push_back(&state.points[o]);
    }
    if (!into && p.y < state.frame.h-1 && p.x < state.frame.w-1) {
      size_t o = getOff(state, p.x+1, p.y+1);
      result.push_back(&state.points[o]);
    }
    if (((state.direction == FLOW_LEFT_RIGHT && !into) ||
         (state.direction == FLOW_TOP_BOTTOM && into)) &&
        p.y > 0 && p.x < state.frame.w - 1) {
      size_t o = getOff(state, p.x+1, p.y-1);
      result.push_back(&state.points[o]);
    }
  } else if (&p == &state.s && into) {
    if (state.direction == FLOW_LEFT_RIGHT) {
      for(size_t i = 0; i < state.frame.h; i++) {
        size_t o = getOff(state, 0, i);
        result.push_back(&state.points[o]);
      }
    } else {
      for(size_t i = 0; i < state.frame.w; i++) {
        size_t o = getOff(state, i, 0);
        result.push_back(&state.points[o]);
      }
    }
  } else if (&p == &state.t && !into) {
    if (state.direction == FLOW_LEFT_RIGHT) {
      for(size_t i = 0; i < state.frame.h; i++) {
        size_t o = getOff(state, state.frame.w-1, i);
        result.push_back(&state.points[o]);
      }
    } else {
      for(size_t i = 0; i < state.frame.w; i++) {
        size_t o = getOff(state, i, state.frame.h-1);
        result.push_back(&state.points[o]);
      }
    }
  }
  return result;
}

inline static unsigned int tree_cap(const Point& from, const Point& to) {
  if (from.next == &to) {
    return from.capacity - from.flow;
  } else {
    return 1;
  }
}

inline static const Point* getOrigin(const Point* p) {
  return p->parent?getOrigin(p->parent):p;
}

inline static void setParent(Point& p, Point& parent) {
  p.parent = &parent;
  p.tree = parent.tree;
  parent.children.push_back(&p);
}

inline static void invalidate(FlowState& state, Point& p) {
  p.parent->children.erase(find(p.parent->children.begin(),
                                p.parent->children.end(), &p));
  p.parent = NULL;
  addOrphan(state, &p);
}

static void adopt(FlowState& state) {
  while (!state.O.empty()) {
    if (DEBUG) {
      cout << "Adopting... (" << state.O.size() << ")\n";
    }
    Point* p = getOrphan(state);

    if (DEBUG) {
      cout << "Looking for parent for (X:" << p->x << ",y:" << p->y;
      cout << " in " << (p->tree==&state.s?"S":"T") << ")\n";
    }

    Point::NeighborSet::iterator i;

    if (p->tree == &state.s) {
      // look for a parent that flows into p
      for(i = p->from.begin(); i != p->from.end(); ++i) {
        if ((*i)->tree == &state.s && tree_cap(**i, *p) &&
            getOrigin(*i) == &state.s) {
          setParent(*p, **i);
          break;
        }
      }
    } else {
      // look for a parent that p flows into
      for(i = p->to.begin(); i != p->to.end(); ++i) {
        if ((*i)->tree == &state.t && tree_cap(*p, **i) &&
            getOrigin(*i) == &state.t) {
          setParent(*p, **i);
          break;
        }
      }
    }
    if (p->parent == NULL) {
      // mark children as orphans
      for (Point::ChildrenSet::iterator j = p->children.begin();
           j != p->children.end(); ++j) {
        (*j)->parent = NULL;
        addOrphan(state, *j);
      }
      p->children.clear();
      if (p->tree == &state.t) {
        // mark potential parents as active
        for (i = p->to.begin(); i != p->to.end(); ++i) {
          if ((*i)->tree == &state.t && tree_cap(*p, **i)) {
            addActive(state, *i);
          }
        }
      } else {
        // mark potential parents as active
        for (i = p->from.begin(); i != p->from.end(); ++i) {
          if ((*i)->tree == &state.s && tree_cap(**i, *p)) {
            addActive(state, *i);
          }
        }
      }
      p->tree = NULL;
      removeActive(state, p);
      if (DEBUG) {
        cout << "Did not find parent...\n";
      }
    } else {
      if (DEBUG) {
        cout << "Found parent (X:" << p->parent->x << ",y:" << p->parent->y<<")\n";
      }
    }
  }
}

static void augment(FlowState& state, Path& P) {
  // use two iterators to support forward iterators (i.e. lists)
  Path::iterator i, j;
  if (DEBUG) {
    if (P.front() != &state.s || P.back() != &state.t) {
      cout << "WHAT THE FUCK INVALID PATH!!!!!!!!!!";
      return;
    }
  }
  unsigned int bottleneck = ~0;
  i = P.begin();
  ++i;
  j = i;
  ++j;
  for(; j != P.end(); ++i, ++j) {
    if (DEBUG) {
      cout << "Point: x: " << (*i)->x << ", y:" << (*i)->y;
      cout << ", flow:" << (*i)->flow << ", cap:" << (*i)->capacity << ", res:";
    }
    if ((*i)->next == *j) {
      unsigned int diff = (*i)->capacity - (*i)->flow;
      if (diff < bottleneck) {
        bottleneck = diff;
      }
      if (DEBUG) {
        cout << diff;
      }
    } else {
      if (DEBUG) {
        cout << "inf";
      }
    }
    if (DEBUG) {
      cout <<"\n";
    }
  }
  if (DEBUG) {
    cout <<"Augmenting (len " << P.size() << " bottleneck ";
    cout << bottleneck << ")\n";
  }
  i = P.begin();
  j = i;
  ++j;
  for (; j!=P.end(); ++i, ++j) {
    if (i == P.begin() || j == P.end() || (*i)->next == *j)
      (*i)->flow += bottleneck;
    if (i != P.begin() && j != P.end() && (*i)->next == *j) {
      if ((*i)->flow == (*i)->capacity) {
        if ((*i)->tree == (*j)->tree && (*i)->tree == &state.s) {
          invalidate(state, **j);
        } else if ((*i)->tree == (*j)->tree && (*i)->tree == &state.t) {
          invalidate(state, **i);
        }
      }
    }
  }
}

static Path grow(FlowState& state) {
  while (state.A.size()) {
    if (DEBUG) {
      cout << "Growing... (active " << state.A.size() << ")\n";
    }
    Point* p = getActive(state);

    vector<Point*>& neighbors = ((p->tree == &state.s)?p->to:p->from);
    for (vector<Point*>::iterator i = neighbors.begin();
         i != neighbors.end(); ++i) {
      if (!(p->tree==&state.s?tree_cap(*p, **i):tree_cap(**i,*p))) continue;
      if ((*i)->tree == NULL) {
        setParent(**i, *p);
        addActive(state, *i);
      } else if ((*i)->tree != p->tree) {
        Path path;
        for(Point* x = (((p->tree==&state.s)?p:*i));
            x != NULL; x = x->parent) {
          path.push_front(x);
        }
        for(Point* x = (((p->tree==&state.t)?p:*i));
            x != NULL; x = x->parent) {
          path.push_back(x);
        }
        return path;
      }
    }
  }
  return Path();
}

FlowState* getBestFlow(const Frame<unsigned char>& frame,
                       FlowDirection direction) {
  FlowState* state = new FlowState(frame);

  state->s.tree = &state->s;
  state->t.tree = &state->t;

  state->direction = direction;

  for(size_t y = 0; y < frame.h; y++) {
    for(size_t x = 0; x < frame.w; x++) {
      state->points.push_back(Point(x, y, frame.values[getOff(*state,x,y)]+1));
    }
  }

  for (FlowState::PointsSet::iterator i = state->points.begin();
       i != state->points.end(); ++i) {
    i->to = getNeighbors(*i, *state, true);
    if (i->to[0] != &state->t)
      i->next = i->to[0];
    i->from = getNeighbors(*i, *state, false);
  }

  state->s.to = getNeighbors(state->s, *state, true);
  state->s.from = getNeighbors(state->s, *state, false);
  state->t.to = getNeighbors(state->t, *state, true);
  state->t.from = getNeighbors(state->t, *state, false);

  addActive(*state, &state->s);
  addActive(*state, &state->t);

  while (true) {
    Path P = grow(*state);
    if (P.empty()) {
      return state;
    }
    augment(*state, P);
    adopt(*state);
  }
}
