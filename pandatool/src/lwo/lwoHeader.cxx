/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lwoHeader.cxx
 * @author drose
 * @date 2001-04-24
 */

#include "lwoHeader.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoHeader::_type_handle;

/**
 *
 */
LwoHeader::
LwoHeader() {
  _valid = false;
  _version = 0.0;
}

/**
 * Reads the data of the chunk in from the given input file, if possible.  The
 * ID and length of the chunk have already been read.  stop_at is the byte
 * position of the file to stop at (based on the current position at
 * in->get_bytes_read()).  Returns true on success, false otherwise.
 */
bool LwoHeader::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _lwid = lin->get_id();

  if (_lwid == IffId("LWO2")) {
    _valid = true;
    _version = 6.0;
  } else if (_lwid == IffId("LWOB")) {
    _valid = true;
    _version = 5.0;
  }

  if (_valid) {
    lin->set_lwo_version(_version);
  }

  read_chunks_iff(lin, stop_at);

  return true;
}

/**
 *
 */
void LwoHeader::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " {\n";
  indent(out, indent_level + 2)
    << "id = " << _lwid << "\n";
  write_chunks(out, indent_level + 2);
  indent(out, indent_level)
    << "}\n";
}
