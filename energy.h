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

enum MaxFlowAlogorithm {
  EDMONDS_KARP,
};

#define DEFAULT_ALGORITHM EDMONDS_KARP

enum FlowDirection {
  FLOW_LEFT_RIGHT,
  FLOW_TOP_BOTTOM
};

class FlowState {
public:
  typedef unsigned short EnergyType;
  FlowDirection direction;
  Frame<PixelValue>* energy;
protected:
  FlowState(FrameWrapper& frame) :
    energy(getDifferential(frame)) { }
public:
  virtual EnergyType calcMaxFlow(FlowDirection direction) = 0;

  virtual FrameWrapper* cutFrame(const FrameWrapper& subject,
                                 FrameWrapper* cut) = 0;

  virtual ~FlowState() {
    delete energy;
  }
};

FlowState* getNewFlowState(FrameWrapper& frame);

#endif
