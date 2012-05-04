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
#ifndef _ENERGY_H
#define _ENERGY_H

#include <deque>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <stack>

#include "frame.h"
#include "diff.h"

typedef unsigned short _FlowStateTimeType;
typedef unsigned short _FlowStateDistType;

#include "point.h"

enum FlowDirection {
  FLOW_LEFT_RIGHT,
  FLOW_TOP_BOTTOM
};

struct FlowState {
public:
  // random access required, vector/deque approx same speed.
  typedef std::vector<Point> PointsSet;
  // operations: add, remove something deque clearly faster
  // queue much faster than stack (algorithmically)
  typedef std::queue<Point*, std::deque<Point*> > ActiveSet;
  // operators: add, remove something deque clearly faster than list
  // potential (small) speedup from using a vector with a large reserved size.
  typedef std::stack<Point*, std::deque<Point*> > OrphanSet;

  typedef _FlowStateTimeType TimeType;
  typedef _FlowStateDistType DistType;

  TimeType time;

  PointsSet points;

  ActiveSet A;
  OrphanSet O;

  Point s;
  Point t;

  FlowDirection direction;
  Frame<PixelValue>* energy;

  FlowState(FrameWrapper& frame) :
    energy(getDifferential(frame)) { }

  void calcBestFlow(FlowDirection direction);

  FrameWrapper* cutFrame(const FrameWrapper& subject,
                         FrameWrapper* cut);

  virtual ~FlowState() {
    delete energy;
  }
};

#endif
