// Filename: stitchCommandReader.h
// Created by:  drose (16Mar00)
// 
////////////////////////////////////////////////////////////////////

#ifndef STITCHCOMMANDREADER_H
#define STITCHCOMMANDREADER_H

#include <pandatoolbase.h>

#include "stitchFile.h"

#include <programBase.h>

////////////////////////////////////////////////////////////////////
// 	 Class : StitchCommandReader
// Description : This specialization of ProgramBase is intended for
//               programs in this directory that read and process a
//               stitch command file.
//////////////////////////////////////////////////////////////////////
class StitchCommandReader : public ProgramBase {
public:
  StitchCommandReader();

protected:
  virtual bool handle_args(Args &args);

protected:
  StitchFile _command_file;
};

#endif


