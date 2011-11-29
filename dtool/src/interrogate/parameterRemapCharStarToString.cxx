// Filename: parameterRemapCharStarToString.cxx
// Created by:  drose (09Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "parameterRemapCharStarToString.h"

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapCharStarToString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapCharStarToString::
ParameterRemapCharStarToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ParameterRemapWCharStarToWString::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ParameterRemapWCharStarToWString::
ParameterRemapWCharStarToWString(CPPType *orig_type) :
  ParameterRemapToWString(orig_type)
{
}
