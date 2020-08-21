/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoSurfaceBlockChannel.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoSurfaceBlockChannel.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoSurfaceBlockChannel::_type_handle;

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoSurfaceBlockChannel::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _channel_id = lin->get_id();

  return true;
}

/**
 *
 */
void LwoSurfaceBlockChannel::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { channel_id = " << _channel_id << " }\n";
}
