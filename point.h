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
#ifndef _POINT_H
#define _POINT_H

#include <cstddef>
#include <vector>

#include "energy.h"

struct Point {
public:
  // Only ever create at beginning. After that only iterate.
  typedef std::vector<Point*> NeighborSet;

  enum Tree {
    TREE_NONE,
    TREE_S,
    TREE_T
  };

  Point* parent;
  _FlowStateEnergyType capacity;
  _FlowStateEnergyType flow;
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
