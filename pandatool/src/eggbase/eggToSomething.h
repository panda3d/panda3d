// Filename: eggToSomething.h
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGTOSOMETHING_H
#define EGGTOSOMETHING_H

#include <pandatoolbase.h>

#include "eggConverter.h"

////////////////////////////////////////////////////////////////////
//       Class : EggToSomething
// Description : This is the general base class for a file-converter
//               program that reads some model file format and
//               generates an egg file.
////////////////////////////////////////////////////////////////////
class EggToSomething : public EggConverter {
public:
  EggToSomething(const string &format_name, 
                 const string &preferred_extension = string(),
                 bool allow_last_param = true,
                 bool allow_stdout = true);

protected:
  virtual bool handle_args(Args &args);
};

#endif


