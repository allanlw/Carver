#ifndef _POINT_H
#define _POINT_H

#include <cstddef>
#include <vector>
#include <set>

struct Point {
public:
  std::size_t x;
  std::size_t y;
  Point* tree;
  Point* parent;
  unsigned int capacity;
  unsigned int flow;
  std::vector<Point*> to; // nodes I flow into
  std::vector<Point*> from; // nodes that flow into me
  Point* next;

  const Point* origin;
  std::size_t distToOrigin;
  std::set<Point*> children; // children in the flow tree

  Point() : x(0), y(0), tree(NULL), parent(NULL), capacity(0),
    flow(0), next(NULL), origin(NULL), distToOrigin(0) {};
  Point(const Point& other) : x(other.x),
    y(other.y), tree(other.tree), parent(other.parent),
    capacity(other.capacity), flow(other.flow), next(other.next),
    origin(other.origin), distToOrigin(other.distToOrigin) {};
  Point(std::size_t x, std::size_t y, unsigned int capacity) :
    x(x), y(y), tree(NULL), parent(NULL), capacity(capacity), flow(0),
    next(NULL), origin(NULL), distToOrigin(0) {};
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
      this->origin = other.origin;
      this->distToOrigin = other.distToOrigin;
    }
    return *this;
  }
};

#endif
