// Filename: eggReader.h
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGREADER_H
#define EGGREADER_H

#include <pandatoolbase.h>

#include "eggBase.h"


////////////////////////////////////////////////////////////////////
// 	 Class : EggReader
// Description : This is the base class for a program that reads egg
//               files, but doesn't write an egg file.
////////////////////////////////////////////////////////////////////
class EggReader : virtual public EggBase {
public:
  EggReader();

protected:
  virtual bool handle_args(Args &args);

  bool _force_complete;
};

#endif


