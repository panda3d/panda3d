// Filename: eggConverter.h
// Created by:  drose (15Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGCONVERTER_H
#define EGGCONVERTER_H

#include <pandatoolbase.h>

#include "eggFilter.h"

////////////////////////////////////////////////////////////////////
// 	 Class : EggConverter
// Description : This is a general base class for programs that
//               convert between egg files and some other format.  See
//               EggToSomething and SomethingToEgg.
////////////////////////////////////////////////////////////////////
class EggConverter : public EggFilter {
public:
  EggConverter(const string &format_name, 
	       const string &preferred_extension = string(),
	       bool allow_last_param = true,
	       bool allow_stdout = true);

protected:
  string _format_name;
};

#endif


