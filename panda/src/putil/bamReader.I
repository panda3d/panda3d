// Filename: bamReader.I
// Created by:  jason (12Jun00)
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


////////////////////////////////////////////////////////////////////
//     Function: BamReader::is_eof
//       Access: Public
//  Description: Returns true if the reader has reached end-of-file,
//               false otherwise.  This call is only valid after a
//               call to read_object().
////////////////////////////////////////////////////////////////////
INLINE bool BamReader::
is_eof() const {
  return _source->is_eof();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_major_ver
//       Access: Public
//  Description: Returns the major version number of the Bam file
//               currently being read.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_major_ver() const {
  return _file_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_minor_ver
//       Access: Public
//  Description: Returns the minor version number of the Bam file
//               currently being read.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_minor_ver() const {
  return _file_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_major_ver
//       Access: Public
//  Description: Returns the major version number of Bam files
//               supported by the current code base.  This must match
//               get_file_major_ver() in order to successfully read a
//               file.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_current_major_ver() const {
  return _cur_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_minor_ver
//       Access: Public
//  Description: Returns the minor version number of Bam files
//               supported by the current code base.  This must match
//               or exceed get_file_minor_ver() in order to
//               successfully read a file.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_current_minor_ver() const {
  return _cur_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_factory
//       Access: Public, Static
//  Description: Returns the global WritableFactory for generating
//               TypedWritable objects
////////////////////////////////////////////////////////////////////
INLINE WritableFactory *BamReader::
get_factory() {
  if (_factory == (WritableFactory *)NULL) {
    create_factory();
  }
  return _factory;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::create_factory
//       Access: Private, Static
//  Description: Creates a new WritableFactory for generating
//               TypedWritable objects
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
create_factory() {
  _factory = new WritableFactory;
}

////////////////////////////////////////////////////////////////////
//     Function: parse_params
//       Access: Private, Static
//  Description: Takes in a FactoryParams, passed from a WritableFactory
//               into any TypedWritable's make function, and parses
//               out the datagram that contatins the data it needs to
//               construct itself, and parses out the pointer to the
//               managing BamReader object
////////////////////////////////////////////////////////////////////
INLINE void
parse_params(const FactoryParams &params,
             DatagramIterator &scan, BamReader *&manager) {
  BamReaderParam *param = DCAST(BamReaderParam, params.get_param(0));

  scan = param->get_iterator();
  manager = param->get_manager();
}
