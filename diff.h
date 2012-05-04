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
#ifndef _DIFF_H
#define _DIFF_H

#include "frame.h"

Frame<PixelValue>* getDifferential(const Frame<PixelValue>& frame);
Frame<PixelValue>* getDifferential(const Frame<RgbPixel>& frame);
Frame<PixelValue>* getDifferential(const FrameWrapper& frame);

void zeroFrame(Frame<PixelValue>& frame);
void zeroFrame(Frame<RgbPixel>& frame);
void zeroFrame(FrameWrapper& frame);

void togglePixel(Frame<PixelValue>& frame, std::size_t x, std::size_t y);
void togglePixel(Frame<RgbPixel>& frame, std::size_t x, std::size_t y);
void togglePixel(FrameWrapper& frame, std::size_t x, std::size_t y);

#endif
