// Filename: eggFilter.h
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGFILTER_H
#define EGGFILTER_H

#include <pandatoolbase.h>

#include "eggReader.h"
#include "eggWriter.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggFilter
// Description : This is the base class for a program that reads an
//               egg file, operates on it, and writes another egg file
//               out.
////////////////////////////////////////////////////////////////////
class EggFilter : public EggReader, public EggWriter {
public:
  EggFilter(bool allow_last_param = false, bool allow_stdout = true);

protected:
  virtual bool handle_args(Args &args);
};

#endif


