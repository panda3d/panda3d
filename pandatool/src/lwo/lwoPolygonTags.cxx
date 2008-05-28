// Filename: lwoPolygonTags.cxx
// Created by:  drose (24Apr01)
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

#include "lwoPolygonTags.h"
#include "lwoInputFile.h"

#include "dcast.h"
#include "indent.h"

TypeHandle LwoPolygonTags::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: LwoPolygonTags::has_tag
//       Access: Public
//  Description: Returns true if the map has a tag associated with
//               the given polygon index, false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoPolygonTags::
has_tag(int polygon_index) const {
  return (_tmap.count(polygon_index) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygonTags::get_tag
//       Access: Public
//  Description: Returns the tag associated with the given polygon
//               index, or -1 if there is no tag associated.
////////////////////////////////////////////////////////////////////
int LwoPolygonTags::
get_tag(int polygon_index) const {
  TMap::const_iterator ti;
  ti = _tmap.find(polygon_index);
  if (ti != _tmap.end()) {
    return (*ti).second;
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygonTags::read_iff
//       Access: Public, Virtual
//  Description: Reads the data of the chunk in from the given input
//               file, if possible.  The ID and length of the chunk
//               have already been read.  stop_at is the byte position
//               of the file to stop at (based on the current position
//               at in->get_bytes_read()).  Returns true on success,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool LwoPolygonTags::
read_iff(IffInputFile *in, size_t stop_at) {
  LwoInputFile *lin = DCAST(LwoInputFile, in);

  _tag_type = lin->get_id();

  while (lin->get_bytes_read() < stop_at && !lin->is_eof()) {
    int polygon_index = lin->get_vx();
    int tag = lin->get_be_int16();

    bool inserted = _tmap.insert(TMap::value_type(polygon_index, tag)).second;
    if (!inserted) {
      nout << "Duplicate index " << polygon_index << " in map.\n";
    }
  }

  return (lin->get_bytes_read() == stop_at);
}

////////////////////////////////////////////////////////////////////
//     Function: LwoPolygonTags::write
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void LwoPolygonTags::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_id() << " { tag_type = " << _tag_type << ", "
    << _tmap.size() << " values }\n";
}
