// Filename: somethingToEgg.h
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOMETHINGTOEGG_H
#define SOMETHINGTOEGG_H

#include <pandatoolbase.h>

#include "eggConverter.h"

////////////////////////////////////////////////////////////////////
// 	 Class : SomethingToEgg
// Description : This is the general base class for a file-converter
//               program that reads some model file format and
//               generates an egg file.
////////////////////////////////////////////////////////////////////
class SomethingToEgg : public EggConverter {
public:
  SomethingToEgg(const string &format_name, 
		 const string &preferred_extension = string(),
		 bool allow_last_param = true,
		 bool allow_stdout = true);

protected:
  virtual bool handle_args(Args &args);

  Filename _input_filename;
};

#endif


