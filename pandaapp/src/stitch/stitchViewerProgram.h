// Filename: stitchViewerProgram.h
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHVIEWERPROGRAM_H
#define STITCHVIEWERPROGRAM_H

#include <pandatoolbase.h>

#include "stitchCommandReader.h"

////////////////////////////////////////////////////////////////////
// 	 Class : StitchViewerProgram
// Description : A program to read a stitch command file, and draw a
//               3-d representation of all of the input images.
////////////////////////////////////////////////////////////////////
class StitchViewerProgram : public StitchCommandReader {
public:
  StitchViewerProgram();

  void run();
};

#endif

