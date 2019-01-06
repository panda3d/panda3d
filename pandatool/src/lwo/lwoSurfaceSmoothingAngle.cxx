/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceSmoothingAngle.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoSurfaceSmoothingAngle.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"
#include "deg_2_rad.h"

TypeHandle LwoSurfaceSmoothingAngle::_type_handle;

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoSurfaceSmoothingAngle::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _angle = lin->get_be_float32();

  return true;
}

/**
 *
 */
void LwoSurfaceSmoothingAngle::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { angle = " << rad_2_deg(_angle) << " degrees }\n";
}
