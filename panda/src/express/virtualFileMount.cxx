// Filename: virtualFileMount.cxx
// Created by:  drose (03Aug02)
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

#include "virtualFileMount.h"

TypeHandle VirtualFileMount::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMount::
~VirtualFileMount() {
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::close_read_file
//       Access: Public
//  Description: Closes a file opened by a previous call to
//               open_read_file().  This really just deletes the
//               istream pointer, but it is recommended to use this
//               interface instead of deleting it explicitly, to help
//               work around compiler issues.
////////////////////////////////////////////////////////////////////
void VirtualFileMount::
close_read_file(istream *stream) const {
  if (stream != (istream *)NULL) {
    // For some reason--compiler bug in gcc 3.2?--explicitly deleting
    // the stream pointer does not call the appropriate global delete
    // function; instead apparently calling the system delete
    // function.  So we call the delete function by hand instead.
#if !defined(USE_MEMORY_NOWRAPPERS) && defined(REDEFINE_GLOBAL_OPERATOR_NEW)
    stream->~istream();
    (*global_operator_delete)(stream);
#else
    delete stream;
#endif
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMount::
output(ostream &out) const {
  out << get_physical_filename();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMount::
write(ostream &out) const {
  out << get_physical_filename() << " on /" << get_mount_point() << "\n";
}
