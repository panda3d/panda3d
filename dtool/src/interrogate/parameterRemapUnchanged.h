// Filename: parameterRemapUnchanged.h
// Created by:  drose (01Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef PARAMETERREMAPUNCHANGED_H
#define PARAMETERREMAPUNCHANGED_H

#include <dtoolbase.h>

#include "parameterRemap.h"

////////////////////////////////////////////////////////////////////
// 	 Class : ParameterRemapUnchanged
// Description : A ParameterRemap class that represents no change to
//               the parameter: the parameter type is legal as is.
////////////////////////////////////////////////////////////////////
class ParameterRemapUnchanged : public ParameterRemap {
public:
  ParameterRemapUnchanged(CPPType *orig_type);
};

#endif
