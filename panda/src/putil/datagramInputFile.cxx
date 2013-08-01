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
#include "temporaryFile.h"
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
//       Access: Published
//  Description: Opens the indicated filename for reading.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
open(const FileReference *file) {
  close();

  _file = file;
  _filename = _file->get_filename();

  // DatagramInputFiles are always binary.
  _filename.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  _vfile = vfs->get_file(_filename);
  if (_vfile == (VirtualFile *)NULL) {
    // No such file.
    return false;
  }
  _timestamp = _vfile->get_timestamp();
  _in = _vfile->open_read_file(true);
  _owns_in = (_in != (istream *)NULL);
  return _owns_in && !_in->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::open
//       Access: Published
//  Description: Starts reading from the indicated stream.  Returns
//               true on success, false on failure.  The
//               DatagramInputFile does not take ownership of the
//               stream; you are responsible for closing or deleting
//               it when you are done.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
open(istream &in, const Filename &filename) {
  close();

  _in = &in;
  _owns_in = false;
  _filename = filename;
  _timestamp = 0;

  if (!filename.empty()) {
    _file = new FileReference(filename);
  }

  return !_in->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::close
//       Access: Published
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramInputFile destructs.
////////////////////////////////////////////////////////////////////
void DatagramInputFile::
close() {
  _vfile.clear();
  if (_owns_in) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_in);
  }
  _in = (istream *)NULL;
  _owns_in = false;

  _file.clear();
  _filename = Filename();
  _timestamp = 0;

  _read_first_datagram = false;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::read_header
//       Access: Published
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
//       Access: Published, Virtual
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
  PN_uint32 num_bytes_32 = reader.get_uint32();
  if (_in->fail() || _in->eof()) {
    return false;
  }

  if (num_bytes_32 == 0) {
    // A special case for a zero-length datagram: no need to try to
    // read any data.
    data.clear();
    return true;
  }

  streamsize num_bytes = (streamsize)num_bytes_32;
  if (num_bytes_32 == (PN_uint32)-1) {
    // Another special case for a value larger than 32 bits.
    num_bytes = reader.get_uint64();
  }

  // Make sure we have a reasonable datagram size for putting into
  // memory.
  nassertr(num_bytes == (size_t)num_bytes, false);

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
//     Function: DatagramInputFile::save_datagram
//       Access: Published, Virtual
//  Description: Skips over the next datagram without extracting it,
//               but saves the relevant file information in the
//               SubfileInfo object so that its data may be read
//               later.  For non-file-based datagram generators, this
//               may mean creating a temporary file and copying the
//               contents of the datagram to disk.
//
//               Returns true on success, false on failure or if this
//               method is unimplemented.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
save_datagram(SubfileInfo &info) {
  nassertr(_in != (istream *)NULL, false);
  _read_first_datagram = true;

  // First, get the size of the upcoming datagram.
  StreamReader reader(_in, false);
  size_t num_bytes_32 = reader.get_uint32();
  if (_in->fail() || _in->eof()) {
    return false;
  }

  streamsize num_bytes = (streamsize)num_bytes_32;
  if (num_bytes_32 == (PN_uint32)-1) {
    // Another special case for a value larger than 32 bits.
    num_bytes = reader.get_uint64();
  }

  // If this stream is file-based, we can just point the SubfileInfo
  // directly into this file.
  if (_file != (FileReference *)NULL) {
    info = SubfileInfo(_file, _in->tellg(), num_bytes);
    _in->seekg(num_bytes, ios::cur);
    return true;
  }

  // Otherwise, we have to dump the data into a temporary file.
  PT(TemporaryFile) tfile = new TemporaryFile(Filename::temporary("", ""));
  pofstream out;
  Filename filename = tfile->get_filename();
  filename.set_binary();
  if (!filename.open_write(out)) {
    util_cat.error()
      << "Couldn't write to " << tfile->get_filename() << "\n";
    return false;
  }

  if (util_cat.is_debug()) {
    util_cat.debug()
      << "Copying " << num_bytes << " bytes to " << tfile->get_filename() << "\n";
  }

  streamsize num_remaining = num_bytes;
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  
  _in->read(buffer, min((streamsize)buffer_size, num_remaining));
  streamsize count = _in->gcount();
  while (count != 0) {
    out.write(buffer, count);
    if (out.fail()) {
      util_cat.error()
        << "Couldn't write " << num_bytes << " bytes to " 
        << tfile->get_filename() << "\n";
      return false;
    }
    num_remaining -= count;
    if (num_remaining == 0) {
      break;
    }
    _in->read(buffer, min((streamsize)buffer_size, num_remaining));
    count = _in->gcount();
  }

  if (num_remaining != 0) {
    util_cat.error()
      << "Truncated data stream.\n";
    return false;
  }

  info = SubfileInfo(tfile, 0, num_bytes);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::is_eof
//       Access: Published, Virtual
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
//       Access: Published, Virtual
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
//     Function: DatagramInputFile::get_filename
//       Access: Published, Virtual
//  Description: Returns the filename that provides the source for
//               these datagrams, if any, or empty string if the
//               datagrams do not originate from a file on disk.
////////////////////////////////////////////////////////////////////
const Filename &DatagramInputFile::
get_filename() {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_timestamp
//       Access: Published, Virtual
//  Description: Returns the on-disk timestamp of the file that was
//               read, at the time it was opened, if that is
//               available, or 0 if it is not.
////////////////////////////////////////////////////////////////////
time_t DatagramInputFile::
get_timestamp() const {
  return _timestamp;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_file
//       Access: Published, Virtual
//  Description: Returns the FileReference that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a file on disk.
////////////////////////////////////////////////////////////////////
const FileReference *DatagramInputFile::
get_file() {
  return _file;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_vfile
//       Access: Published, Virtual
//  Description: Returns the VirtualFile that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a VirtualFile.
////////////////////////////////////////////////////////////////////
VirtualFile *DatagramInputFile::
get_vfile() {
  return _vfile;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_file_pos
//       Access: Published, Virtual
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
