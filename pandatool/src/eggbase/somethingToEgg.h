// Filename: somethingToEgg.h
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef SOMETHINGTOEGG_H
#define SOMETHINGTOEGG_H

#include <pandatoolbase.h>

#include "eggConverter.h"

#include <distanceUnit.h>


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
  void apply_units_scale(EggData &data);

  virtual bool handle_args(Args &args);
  virtual void post_process_egg_file();

  Filename _input_filename;

  DistanceUnit _input_units;
  DistanceUnit _output_units;
};

#endif


