#ifndef _ENERGY_H
#define _ENERGY_H

#include <deque>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <stack>

#include "frame.h"

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
  const Frame<PixelValue>& frame;

  FlowState(const Frame<PixelValue>& frame) : frame(frame) {}
};

#include "point.h"

FlowState* getBestFlow(const Frame<PixelValue>& frame,
                       FlowDirection direction);

FrameWrapper* cutFrame(FlowState& state, const FrameWrapper& subject,
                       FrameWrapper* cut);

#endif
