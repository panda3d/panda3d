// Filename: parameterRemapCharStarToString.h
// Created by:  drose (09Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPCHARSTARTOSTRING_H
#define PARAMETERREMAPCHARSTARTOSTRING_H

#include <dtoolbase.h>

#include "parameterRemapToString.h"

////////////////////////////////////////////////////////////////////
//       Class : ParameterRemapCharStarToString
// Description : Maps from (char *) or (const char *) to the atomic
//               string type.
////////////////////////////////////////////////////////////////////
class ParameterRemapCharStarToString : public ParameterRemapToString {
public:
  ParameterRemapCharStarToString(CPPType *orig_type);
};

#endif
