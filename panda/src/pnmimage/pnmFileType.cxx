// Filename: pnmFileType.cxx
// Created by:  drose (15Jun00)
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

#include "pnmFileType.h"

#include "string_utils.h"
#include "executionEnvironment.h"
#include "bamReader.h"
#include "bamWriter.h"

bool PNMFileType::_did_init_pnm = false;
TypeHandle PNMFileType::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileType::
PNMFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PNMFileType::
~PNMFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::get_num_extensions
//       Access: Published, Virtual
//  Description: Returns the number of different possible filename
//               extensions associated with this particular file type.
////////////////////////////////////////////////////////////////////
int PNMFileType::
get_num_extensions() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::get_extension
//       Access: Published, Virtual
//  Description: Returns the nth possible filename extension
//               associated with this particular file type, without a
//               leading dot.
////////////////////////////////////////////////////////////////////
string PNMFileType::
get_extension(int) const {
  nassertr(false, string());
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::get_suggested_extension
//       Access: Published, Virtual
//  Description: Returns a suitable filename extension (without a
//               leading dot) to suggest for files of this type, or
//               empty string if no suggestions are available.
////////////////////////////////////////////////////////////////////
string PNMFileType::
get_suggested_extension() const {
  if (get_num_extensions() > 0) {
    return get_extension(0);
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::has_magic_number
//       Access: Public, Virtual
//  Description: Returns true if this particular file type uses a
//               magic number to identify it, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileType::
has_magic_number() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::matches_magic_number
//       Access: Public, Virtual
//  Description: Returns true if the indicated "magic number" byte
//               stream (the initial few bytes read from the file)
//               matches this particular file type, false otherwise.
////////////////////////////////////////////////////////////////////
bool PNMFileType::
matches_magic_number(const string &) const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::make_reader
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMReader suitable for
//               reading from this file type, if possible.  If reading
//               from this file type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMReader *PNMFileType::
make_reader(istream *, bool, const string &) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::make_writer
//       Access: Public, Virtual
//  Description: Allocates and returns a new PNMWriter suitable for
//               reading from this file type, if possible.  If writing
//               files of this type is not supported, returns NULL.
////////////////////////////////////////////////////////////////////
PNMWriter *PNMFileType::
make_writer(ostream *, bool) {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::init_pnm
//       Access: Protected, Static
//  Description: Initializes the underlying PNM library, if it has not
//               already been initialized.  This should be called by
//               every implementation of make_reader() and
//               make_writer(), to ensure that the library is properly
//               initialized before any I/O is attempted.
////////////////////////////////////////////////////////////////////
void PNMFileType::
init_pnm() {
  if (!_did_init_pnm) {
    _did_init_pnm = true;

    // No reason to do anything here nowadays.
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PNMFileType::write_datagram
//       Access: Public, Virtual
//  Description: Fills the indicated datagram up with a binary
//               representation of the current object, in preparation
//               for writing to a Bam file.
//
//               None of the particular PNMFileType objects store any
//               extra data--at least, not yet--so we just define this
//               up here to do nothing.
////////////////////////////////////////////////////////////////////
void PNMFileType::
write_datagram(BamWriter *, Datagram &) {
}
