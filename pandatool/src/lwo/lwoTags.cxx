// Filename: lwoTags.cxx
// Created by:  drose (24Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "lwoTags.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoTags::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LwoTags::get_num_tags
//       Access: Public
//  Description: Returns the number of tags of this group.
////////////////////////////////////////////////////////////////////
int LwoTags::
get_num_tags() const {
  return _tags.size();
}

////////////////////////////////////////////////////////////////////
//     Function: LwoTags::get_tag
//       Access: Public
//  Description: Returns the nth tag of this group.
////////////////////////////////////////////////////////////////////
string LwoTags::
get_tag(int n) const {
  nassertr(n >= 0 && n < (int)_tags.size(), string());
  return _tags[n];
}

////////////////////////////////////////////////////////////////////
//     Function: LwoTags::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoTags::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
    string tag = lin->get_string();
    _tags.push_back(tag);
  }

  return (lin->get_bytes_read() == stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoTags::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LwoTags::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { ";

  if (!_tags.empty()) {
    Tags::const_iterator ti = _tags.begin();
    out << '"' << *ti << '"';
    ++ti;
    while (ti != _tags.end()) {
      out << ", \"" << *ti << '"';
      ++ti;
    }
  }
  out << " }\n";
}
