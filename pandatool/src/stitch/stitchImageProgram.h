// Filename: stitchImageProgram.h
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHIMAGEPROGRAM_H
#define STITCHIMAGEPROGRAM_H

#include <pandatoolbase.h>

#include "stitchCommandReader.h"

////////////////////////////////////////////////////////////////////
// 	 Class : StitchImageProgram
// Description : A program to read a stitch command file, perform the
//               image manipulations in the CPU, and write output
//               images for each processed image.
////////////////////////////////////////////////////////////////////
class StitchImageProgram : public StitchCommandReader {
public:
  StitchImageProgram();

  void run();
};

#endif

