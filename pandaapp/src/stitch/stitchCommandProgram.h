// Filename: stitchCommandProgram.h
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHCOMMANDPROGRAM_H
#define STITCHCOMMANDPROGRAM_H

#include <pandatoolbase.h>

#include "stitchCommandReader.h"

////////////////////////////////////////////////////////////////////
//       Class : StitchCommandProgram
// Description : A program to read a stitch command file, process it
//               without actually manipulating any images, and write
//               the processed command file out.
////////////////////////////////////////////////////////////////////
class StitchCommandProgram : public StitchCommandReader {
public:
  StitchCommandProgram();

  void run();
};

#endif

