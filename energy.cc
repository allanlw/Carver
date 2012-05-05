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
#include <iostream>

#include "edmondskarp/edmondskarpenergy.h"
#include "pushrelabel/pushrelabelenergy.h"

using namespace std;

FlowState* getNewFlowState(FrameWrapper& frame) {
  switch (DEFAULT_ALGORITHM) {
  case EDMONDS_KARP:
    return new EdmondsKarpFlowState(frame);
  case PUSH_RELABEL:
    return new PushRelabelFlowState(frame);
  default:
    return NULL;
  }
}

FrameWrapper* FlowState::cutFrame(const FrameWrapper& subject,
                                  FrameWrapper* cut) {
  if ((subject.getWidth() != this->energy->w ||
       subject.getHeight() != this->energy->h) ||
      (cut != NULL && (cut->getWidth() != subject.getWidth() ||
                       cut->getHeight() != subject.getHeight()))) {
    return NULL;
  }

  FrameWrapper* result = new FrameWrapper(subject.color);
  if (direction == FLOW_LEFT_RIGHT) {
    result->setSize(subject.getWidth()-1, subject.getHeight());
  } else {
    result->setSize(subject.getWidth(), subject.getHeight()-1);
  }

  Frame<PixelValue>* newEnergy = new Frame<PixelValue>(result->getWidth(),
                                                       result->getHeight());

  if (cut != NULL) {
    zeroFrame(*cut);
  }

  for(size_t i = 0; i < points.size(); i++) {
    std::size_t x = i % subject.getWidth(), y = i / subject.getWidth();
    std::size_t tox, toy;
    if (points[i].tree == Point::TREE_T ||
        points[i].tree == Point::TREE_NONE) {
      if (direction == FLOW_LEFT_RIGHT && x > 0) {
        tox = x - 1;
        toy = y;
      } else if (direction == FLOW_TOP_BOTTOM && y > 0) {
        tox = x;
        toy = y - 1;
      } else {
        tox = x;
        toy = y;
      }
    } else {
      tox = x;
      toy = y;
    }
    if (subject.color) {
      result->colorFrame->values[tox + toy * result->getWidth()] =
        subject.colorFrame->values[x + y * subject.getWidth()];
    } else {
      result->greyFrame->values[tox + toy * result->getWidth()] =
        subject.greyFrame->values[x + y * subject.getWidth()];
    }
    newEnergy->values[tox + toy * newEnergy->w] =
      this->energy->values[x + y * this->energy->w];
    if (cut != NULL) {
      togglePixel(*cut, tox, toy);
    }
  }
  delete this->energy;
  this->energy = newEnergy;
  return result;
}
