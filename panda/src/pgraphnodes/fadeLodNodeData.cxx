/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fadeLodNodeData.cxx
 * @author drose
 * @date 2004-09-29
 */

#include "fadeLodNodeData.h"

TypeHandle FadeLODNodeData::_type_handle;


/**
 *
 */
void FadeLODNodeData::
output(std::ostream &out) const {
  AuxSceneData::output(out);
  if (_fade_mode != FM_solid) {
    out << " fading " << _fade_out << " to " << _fade_in << " since "
        << _fade_start;
  } else {
    out << " showing " << _fade_in;
  }
}
