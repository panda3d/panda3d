/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file imageBase.cxx
 * @author drose
 * @date 2000-06-19
 */

#include "imageBase.h"

/**

 */
ImageBase::
ImageBase() {
}


/**

 */
bool ImageBase::
post_command_line() {
  return ProgramBase::post_command_line();
}
