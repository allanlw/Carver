#ifndef _DIFF_H
#define _DIFF_H

#include "frame.h"

Frame<unsigned char>* getDifferential(const Frame<unsigned char>& frame);
Frame<unsigned char>* getDifferential(const Frame<RgbPixel>& frame);
Frame<unsigned char>* getDifferential(const FrameWrapper& frame);

void zeroFrame(Frame<unsigned char>& frame);
void zeroFrame(Frame<RgbPixel>& frame);
void zeroFrame(FrameWrapper& frame);

void togglePixel(Frame<unsigned char>& frame, std::size_t x, std::size_t y);
void togglePixel(Frame<RgbPixel>& frame, std::size_t x, std::size_t y);
void togglePixel(FrameWrapper& frame, std::size_t x, std::size_t y);

#endif
