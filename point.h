#ifndef _POINT_H
#define _POINT_H

#include <cstddef>
#include <vector>
#include <list>

#include "energy.h"

struct Point {
public:
  // Only ever create at beginning. After that only iterate.
  typedef std::vector<Point*> NeighborSet;

  typedef unsigned short EnergyType;

  enum Tree {
    TREE_NONE,
    TREE_S,
    TREE_T
  };

  Point* parent;
  EnergyType capacity;
  EnergyType flow;
  NeighborSet to; // nodes I flow into
  NeighborSet from; // nodes that flow into me
  Point* next;
  Tree tree;
  bool active;

  _FlowStateDistType dist;
  _FlowStateTimeType time;

  Point() : parent(NULL), capacity(0),
    flow(0), to(), from(), next(NULL), tree(TREE_NONE),
    active(false), dist(0), time(0) {};
  Point(const Point& other) : parent(other.parent),
    capacity(other.capacity), flow(other.flow), to(other.to),
    from(other.from), next(other.next), tree(other.tree),
    active(other.active), dist(other.dist), time(other.time) {};
  Point& operator=(const Point& other) {
    if (&other != this) {
      this->parent = other.parent;
      this->capacity = other.capacity;
      this->flow = other.flow;
      this->to = other.to;
      this->from = other.from;
      this->next = other.next;
      this->tree = other.tree;
      this->active = other.active;
      this->dist = other.dist;
      this->time = other.time;
    }
    return *this;
  }
};

#endif
