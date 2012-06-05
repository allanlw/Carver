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
#include "pushrelabel.h"

using namespace std;

FlowState::EnergyType PushRelabelFlowState::calcMaxFlow(FlowDirection direction) {
  this->direction = direction;

  A = ActiveSet();
  O = OrphanSet();

  s = Point();
  t = Point();

  s.dist = t.dist = 0;

  points.clear();
  points.resize(energy->h * energy->w);

  if (direction == FLOW_LEFT_RIGHT) {
    buildGraph<FLOW_LEFT_RIGHT>(*this);
  } else {
    buildGraph<FLOW_TOP_BOTTOM>(*this);
  }

  while (true) {
/*
    Path* P = grow(*this);
    if (P == NULL) {
      return s.flow;
    }

    time += 1;
    // hopefully this should never happen, but if it does...
    if (time == 0) {
      for (EdmondsKarpFlowState::PointsSet::iterator i = points.begin();
           i != points.end(); ++i) {
        i->time = 0;
        i->dist = 0;
      }
      time += 1;
    }
    s.time = t.time = time;

    augment(*this, *P);
    delete P;
    adopt(*this);
*/
  }
}
