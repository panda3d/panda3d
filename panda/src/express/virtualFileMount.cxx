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
#include "virtualFileSimple.h"

TypeHandle VirtualFileMount::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
VirtualFileMount::
~VirtualFileMount() {
  nassertv(_file_system == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::make_virtual_file
//       Access: Public, Virtual
//  Description: Constructs and returns a new VirtualFile instance
//               that corresponds to the indicated filename within
//               this mount point.  The returned VirtualFile object
//               does not imply that the given file actually exists;
//               but if the file does exist, then the handle can be
//               used to read it.
////////////////////////////////////////////////////////////////////
PT(VirtualFile) VirtualFileMount::
make_virtual_file(const Filename &local_filename,
                  const Filename &original_filename, bool implicit_pz_file,
                  bool) {
  Filename local(local_filename);
  if (original_filename.is_text()) {
    local.set_text();
  }
  PT(VirtualFileSimple) file =
    new VirtualFileSimple(this, local, implicit_pz_file);
  file->set_original_filename(original_filename);

  return file.p();
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
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: VirtualFileMount::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void VirtualFileMount::
write(ostream &out) const {
  out << *this << " on /" << get_mount_point() << "\n";
}
