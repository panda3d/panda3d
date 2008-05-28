// Filename: datagramInputFile.cxx
// Created by:  drose (30Oct00)
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

#include "datagramInputFile.h"
#include "numeric_types.h"
#include "datagramIterator.h"
#include "profileTimer.h"
#include "config_util.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "streamReader.h"
#include "thread.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::open
//       Access: Public
//  Description: Opens the indicated filename for reading.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
open(Filename filename) {
  close();

  // DatagramInputFiles are always binary.
  filename.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  _vfile = vfs->get_file(filename);
  if (_vfile == (VirtualFile *)NULL) {
    // No such file.
    return false;
  }
  _in = _vfile->open_read_file(true);
  _owns_in = (_in != (istream *)NULL);
  return _owns_in && !_in->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::open
//       Access: Public
//  Description: Starts reading from the indicated stream.  Returns
//               true on success, false on failure.  The
//               DatagramInputFile does not take ownership of the
//               stream; you are responsible for closing or deleting
//               it when you are done.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
open(istream &in) {
  close();

  _in = &in;
  _owns_in = false;

  return !_in->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramInputFile destructs.
////////////////////////////////////////////////////////////////////
void DatagramInputFile::
close() {
  _vfile.clear();
  _in_file.close();
  if (_owns_in) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_in);
  }
  _in = (istream *)NULL;
  _owns_in = false;

  _read_first_datagram = false;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::read_header
//       Access: Public
//  Description: Reads a sequence of bytes from the beginning of the
//               datagram file.  This may be called any number of
//               times after the file has been opened and before the
//               first datagram is read.  It may not be called once
//               the first datagram has been read.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
read_header(string &header, size_t num_bytes) {
  nassertr(!_read_first_datagram, false);
  nassertr(_in != (istream *)NULL, false);

  char *buffer = (char *)alloca(num_bytes);
  nassertr(buffer != (char *)NULL, false);

  _in->read(buffer, num_bytes);
  if (_in->fail() || _in->eof()) {
    return false;
  }

  header = string(buffer, num_bytes);
  Thread::consider_yield();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_datagram
//       Access: Public, Virtual
//  Description: Reads the next datagram from the file.  Returns true
//               on success, false if there is an error or end of
//               file.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
get_datagram(Datagram &data) {
  nassertr(_in != (istream *)NULL, false);
  _read_first_datagram = true;

  // First, get the size of the upcoming datagram.
  StreamReader reader(_in, false);
  PN_uint32 num_bytes = reader.get_uint32();
  if (_in->fail() || _in->eof()) {
    return false;
  }

  if (num_bytes == 0) {
    // A special case for a zero-length datagram: no need to try to
    // read any data.
    data.clear();
    return true;
  }

  // Now, read the datagram itself.

  // If the number of bytes is large, we will need to allocate a
  // temporary buffer from the heap.  Otherwise, we can get away with
  // allocating it on the stack, via alloca().
  if (num_bytes > 65536) {
    char *buffer = (char *)PANDA_MALLOC_ARRAY(num_bytes);
    nassertr(buffer != (char *)NULL, false);

    _in->read(buffer, num_bytes);
    if (_in->fail() || _in->eof()) {
      _error = true;
      PANDA_FREE_ARRAY(buffer);
      return false;
    }

    data = Datagram(buffer, num_bytes);
    PANDA_FREE_ARRAY(buffer);

  } else {
    char *buffer = (char *)alloca(num_bytes);
    nassertr(buffer != (char *)NULL, false);

    _in->read(buffer, num_bytes);
    if (_in->fail() || _in->eof()) {
      _error = true;
      return false;
    }

    data = Datagram(buffer, num_bytes);
  }
  Thread::consider_yield();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::is_eof
//       Access: Public, Virtual
//  Description: Returns true if the file has reached the end-of-file.
//               This test may only be made after a call to
//               read_header() or get_datagram() has failed.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
is_eof() {
  return _in != (istream *)NULL ? _in->eof() : true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::is_error
//       Access: Public, Virtual
//  Description: Returns true if the file has reached an error
//               condition.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
is_error() {
  if (_in == (istream *)NULL) {
    return true;
  }

  if (_in->fail()) {
    _error = true;
  }
  return _error;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_file
//       Access: Public, Virtual
//  Description: Returns the VirtualFile that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a VirtualFile.
////////////////////////////////////////////////////////////////////
VirtualFile *DatagramInputFile::
get_file() {
  return _vfile;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_file_pos
//       Access: Public, Virtual
//  Description: Returns the current file position within the data
//               stream, if any, or 0 if the file position is not
//               meaningful or cannot be determined.
//
//               For DatagramInputFiles that return a meaningful file
//               position, this will be pointing to the first byte
//               following the datagram returned after a call to
//               get_datagram().
////////////////////////////////////////////////////////////////////
streampos DatagramInputFile::
get_file_pos() {
  if (_in == (istream *)NULL) {
    return 0;
  }
  return _in->tellg();
}
