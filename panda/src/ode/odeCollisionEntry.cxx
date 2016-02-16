/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeCollisionEntry.cxx
 * @author rdb
 * @date 2009-03-05
 */

#include "odeCollisionEntry.h"

TypeHandle OdeCollisionEntry::_type_handle;

/**

 */
OdeCollisionEntry::
~OdeCollisionEntry() {
  delete[] _contact_geoms;
}
