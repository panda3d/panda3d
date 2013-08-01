// Filename: datagramOutputFile.cxx
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

#include "datagramOutputFile.h"
#include "streamWriter.h"
#include "zStream.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::open
//       Access: Public
//  Description: Opens the indicated filename for writing.  Returns
//               true if successful, false on failure.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
open(const FileReference *file) {
  close();

  _file = file;
  _filename = _file->get_filename();

  // DatagramOutputFiles are always binary.
  _filename.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  _vfile = vfs->create_file(_filename);
  if (_vfile == (VirtualFile *)NULL) {
    // No such file.
    return false;
  }
  _out = _vfile->open_write_file(true, true);
  _owns_out = (_out != (ostream *)NULL);
  return _owns_out && !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::open
//       Access: Public
//  Description: Starts writing to the indicated stream.  Returns
//               true on success, false on failure.  The
//               DatagramOutputFile does not take ownership of the
//               stream; you are responsible for closing or deleting
//               it when you are done.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
open(ostream &out, const Filename &filename) {
  close();

  _out = &out;
  _owns_out = false;
  _filename = filename;

  if (!filename.empty()) {
    _file = new FileReference(filename);
  }

  return !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::close
//       Access: Public
//  Description: Closes the file.  This is also implicitly done when
//               the DatagramOutputFile destructs.
////////////////////////////////////////////////////////////////////
void DatagramOutputFile::
close() {
  _vfile.clear();
  if (_owns_out) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_write_file(_out);
  }
  _out = (ostream *)NULL;
  _owns_out = false;

  _file.clear();
  _filename = Filename();

  _wrote_first_datagram = false;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::write_header
//       Access: Public
//  Description: Writes a sequence of bytes to the beginning of the
//               datagram file.  This may be called any number of
//               times after the file has been opened and before the
//               first datagram is written.  It may not be called once
//               the first datagram is written.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
write_header(const string &header) {
  nassertr(_out != (ostream *)NULL, false);
  nassertr(!_wrote_first_datagram, false);

  _out->write(header.data(), header.size());
  thread_consider_yield();
  return !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::put_datagram
//       Access: Public, Virtual
//  Description: Writes the given datagram to the file.  Returns true
//               on success, false if there is an error.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
put_datagram(const Datagram &data) {
  nassertr(_out != (ostream *)NULL, false);
  _wrote_first_datagram = true;

  // First, write the size of the upcoming datagram.
  StreamWriter writer(_out, false);
  size_t num_bytes = data.get_length();
  if (num_bytes == (PN_uint32)-1 || num_bytes != (PN_uint32)num_bytes) {
    // Write a large value as a 64-bit size.
    writer.add_uint32((PN_uint32)-1);
    writer.add_uint64(num_bytes);
  } else {
    // Write a value that fits in 32 bits.
    writer.add_uint32((PN_uint32)num_bytes);
  }

  // Now, write the datagram itself.
  _out->write((const char *)data.get_data(), data.get_length());
  thread_consider_yield();

  return !_out->fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::copy_datagram
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
bool DatagramOutputFile::
copy_datagram(SubfileInfo &result, const Filename &filename) {
  nassertr(_out != (ostream *)NULL, false);
  _wrote_first_datagram = true;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  PT(VirtualFile) vfile = vfs->get_file(filename);
  if (vfile == NULL) {
    return false;
  }
  istream *in = vfile->open_read_file(true);
  if (in == NULL) {
    return false;
  }

  streamsize size = vfile->get_file_size(in);
  streamsize num_remaining = size;

  StreamWriter writer(_out, false);
  if (num_remaining == (PN_uint32)-1 || num_remaining != (PN_uint32)num_remaining) {
    // Write a large value as a 64-bit size.
    writer.add_uint32((PN_uint32)-1);
    writer.add_uint64(num_remaining);
  } else {
    // Write a value that fits in 32 bits.
    writer.add_uint32((PN_uint32)num_remaining);
  }

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];

  streampos start = _out->tellp();
  in->read(buffer, min((streamsize)buffer_size, num_remaining));
  streamsize count = in->gcount();
  while (count != 0) {
    _out->write(buffer, count);
    if (_out->fail()) {
      vfile->close_read_file(in);
      return false;
    }
    num_remaining -= count;
    if (num_remaining == 0) {
      break;
    }
    in->read(buffer, min((streamsize)buffer_size, num_remaining));
    count = in->gcount();
  }

  vfile->close_read_file(in);

  if (num_remaining != 0) {
    util_cat.error()
      << "Truncated input stream.\n";
    return false;
  }
  
  result = SubfileInfo(_file, start, size);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::copy_datagram
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
bool DatagramOutputFile::
copy_datagram(SubfileInfo &result, const SubfileInfo &source) {
  nassertr(_out != (ostream *)NULL, false);
  _wrote_first_datagram = true;

  pifstream in;
  if (!source.get_filename().open_read(in)) {
    return false;
  }

  streamsize num_remaining = source.get_size();

  StreamWriter writer(_out, false);
  if (num_remaining == (PN_uint32)-1 || num_remaining != (PN_uint32)num_remaining) {
    // Write a large value as a 64-bit size.
    writer.add_uint32((PN_uint32)-1);
    writer.add_uint64(num_remaining);
  } else {
    // Write a value that fits in 32 bits.
    writer.add_uint32((PN_uint32)num_remaining);
  }

  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  
  streampos start = _out->tellp();
  in.seekg(source.get_start());
  in.read(buffer, min((streamsize)buffer_size, num_remaining));
  streamsize count = in.gcount();
  while (count != 0) {
    _out->write(buffer, count);
    if (_out->fail()) {
      return false;
    }
    num_remaining -= count;
    if (num_remaining == 0) {
      break;
    }
    in.read(buffer, min((streamsize)buffer_size, num_remaining));
    count = in.gcount();
  }

  if (num_remaining != 0) {
    util_cat.error()
      << "Truncated input stream.\n";
    return false;
  }

  result = SubfileInfo(_file, start, source.get_size());
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::is_error
//       Access: Public, Virtual
//  Description: Returns true if the file has reached an error
//               condition.
////////////////////////////////////////////////////////////////////
bool DatagramOutputFile::
is_error() {
  if (_out == (ostream *)NULL) {
    return true;
  }

  if (_out->fail()) {
    _error = true;
  }
  return _error;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::flush
//       Access: Public, Virtual
//  Description: Ensures that all datagrams previously written will be
//               visible in the output file.
////////////////////////////////////////////////////////////////////
void DatagramOutputFile::
flush() {
  if (_out != (ostream *)NULL) {
    _out->flush();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::get_filename
//       Access: Published, Virtual
//  Description: Returns the filename that provides the target for
//               these datagrams, if any, or empty string if the
//               datagrams do not get written to a file on disk.
////////////////////////////////////////////////////////////////////
const Filename &DatagramOutputFile::
get_filename() {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::get_file
//       Access: Published, Virtual
//  Description: Returns the FileReference that provides the target for
//               these datagrams, if any, or NULL if the datagrams do
//               not written to a file on disk.
////////////////////////////////////////////////////////////////////
const FileReference *DatagramOutputFile::
get_file() {
  return _file;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramOutputFile::get_file_pos
//       Access: Published, Virtual
//  Description: Returns the current file position within the data
//               stream, if any, or 0 if the file position is not
//               meaningful or cannot be determined.
//
//               For DatagramOutputFiles that return a meaningful file
//               position, this will be pointing to the first byte
//               following the datagram returned after a call to
//               put_datagram().
////////////////////////////////////////////////////////////////////
streampos DatagramOutputFile::
get_file_pos() {
  if (_out == (ostream *)NULL) {
    return 0;
  }
  return _out->tellp();
}
