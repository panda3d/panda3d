// Filename: datagramSink.cxx
// Created by:  jason (07Jun00)
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

#include "datagramSink.h"
#include "fileReference.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramSink::Destructor
//       Access: Public, Virtual
//  Description: Does nothing since this is class is just
//               the definition of an interface
////////////////////////////////////////////////////////////////////
DatagramSink::
~DatagramSink() {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramSink::copy_datagram
//       Access: Published, Virtual
//  Description: Copies the file data from the entire indicated
//               file (via the vfs) as the next datagram.  This is
//               intended to support potentially very large datagrams.
//
//               Returns true on success, false on failure or if this
//               method is unimplemented.  On true, fills "result"
//               with the information that references the copied file,
//               if possible.
////////////////////////////////////////////////////////////////////
bool DatagramSink::
copy_datagram(SubfileInfo &result, const Filename &filename) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramSink::copy_datagram
//       Access: Published, Virtual
//  Description: Copies the file data from the range of the indicated
//               file (outside of the vfs) as the next datagram.  This
//               is intended to support potentially very large
//               datagrams.
//
//               Returns true on success, false on failure or if this
//               method is unimplemented.  On true, fills "result"
//               with the information that references the copied file,
//               if possible.
////////////////////////////////////////////////////////////////////
bool DatagramSink::
copy_datagram(SubfileInfo &result, const SubfileInfo &source) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramSink::get_filename
//       Access: Published, Virtual
//  Description: Returns the filename that provides the target for
//               these datagrams, if any, or empty string if the
//               datagrams do not get written to a file on disk.
////////////////////////////////////////////////////////////////////
const Filename &DatagramSink::
get_filename() {
  const FileReference *file = get_file();
  if (file != (FileReference *)NULL) {
    return file->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramSink::get_file
//       Access: Published, Virtual
//  Description: Returns the FileReference that provides the target for
//               these datagrams, if any, or NULL if the datagrams do
//               not written to a file on disk.
////////////////////////////////////////////////////////////////////
const FileReference *DatagramSink::
get_file() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramSink::get_file_pos
//       Access: Published, Virtual
//  Description: Returns the current file position within the data
//               stream, if any, or 0 if the file position is not
//               meaningful or cannot be determined.
//
//               For DatagramSinks that return a meaningful file
//               position, this will be pointing to the first byte
//               following the datagram returned after a call to
//               put_datagram().
////////////////////////////////////////////////////////////////////
streampos DatagramSink::
get_file_pos() {
  return 0;
}
