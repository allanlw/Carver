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
