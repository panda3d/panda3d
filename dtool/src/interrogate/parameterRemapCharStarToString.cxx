/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file parameterRemapCharStarToString.cxx
 * @author drose
 * @date 2000-08-09
 */

#include "parameterRemapCharStarToString.h"

/**
 *
 */
ParameterRemapCharStarToString::
ParameterRemapCharStarToString(CPPType *orig_type) :
  ParameterRemapToString(orig_type)
{
}

/**
 *
 */
ParameterRemapWCharStarToWString::
ParameterRemapWCharStarToWString(CPPType *orig_type) :
  ParameterRemapToWString(orig_type)
{
}
