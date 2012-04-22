#ifndef _DIFF_H
#define _DIFF_H

#include "frame.h"

Frame<unsigned char>* getDifferential(const Frame<unsigned char>& frame);
Frame<unsigned char>* getDifferential(const Frame<RgbPixel>& frame);

#endif
