#ifndef _POINT_H
#define _POINT_H

#include <cstddef>
#include <vector>
#include <set>
#include <deque>
#include <list>

struct Point {
public:
  typedef std::vector<Point*> NeighborSet;
  typedef std::list<Point*> ChildrenSet;

  std::size_t x;
  std::size_t y;
  Point* tree;
  Point* parent;
  unsigned int capacity;
  unsigned int flow;
  NeighborSet to; // nodes I flow into
  NeighborSet from; // nodes that flow into me
  Point* next;

  ChildrenSet children; // children in the flow tree

  Point() : x(0), y(0), tree(NULL), parent(NULL), capacity(0),
    flow(0), next(NULL) {};
  Point(const Point& other) : x(other.x),
    y(other.y), tree(other.tree), parent(other.parent),
    capacity(other.capacity), flow(other.flow), to(),
    from(), next(other.next), children() {};
  Point(std::size_t x, std::size_t y, unsigned int capacity) :
    x(x), y(y), tree(NULL), parent(NULL), capacity(capacity), flow(0),
    to(), from(), next(NULL), children() {};
  Point& operator=(const Point& other) {
    if (&other != this) {
      this->x = other.x;
      this->y = other.y;
      this->tree = other.tree;
      this->parent = other.parent;
      this->capacity = other.capacity;
      this->flow = other.flow;
      this->to = other.to;
      this->from = other.from;
      this->next = other.next;
      this->children = children;
    }
    return *this;
  }
};

#endif
