/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramInputFile.cxx
 * @author drose
 * @date 2000-10-30
 */

#include "datagramInputFile.h"
#include "temporaryFile.h"
#include "numeric_types.h"
#include "datagramIterator.h"
#include "config_putil.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#include "streamReader.h"
#include "thread.h"

using std::streampos;
using std::streamsize;

/**
 * Opens the indicated filename for reading.  Returns true on success, false
 * on failure.
 */
bool DatagramInputFile::
open(const FileReference *file) {
  close();

  _file = file;
  _filename = _file->get_filename();

  // DatagramInputFiles are always binary.
  _filename.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  _vfile = vfs->get_file(_filename);
  if (_vfile == nullptr) {
    // No such file.
    return false;
  }
  _timestamp = _vfile->get_timestamp();
  _in = _vfile->open_read_file(true);
  _owns_in = (_in != nullptr);
  return _owns_in && !_in->fail();
}

/**
 * Starts reading from the indicated stream.  Returns true on success, false
 * on failure.  The DatagramInputFile does not take ownership of the stream;
 * you are responsible for closing or deleting it when you are done.
 */
bool DatagramInputFile::
open(std::istream &in, const Filename &filename) {
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

/**
 * Closes the file.  This is also implicitly done when the DatagramInputFile
 * destructs.
 */
void DatagramInputFile::
close() {
  _vfile.clear();
  if (_owns_in) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_in);
  }
  _in = nullptr;
  _owns_in = false;

  _file.clear();
  _filename = Filename();
  _timestamp = 0;

  _read_first_datagram = false;
  _error = false;
}

/**
 * Reads a sequence of bytes from the beginning of the datagram file.  This
 * may be called any number of times after the file has been opened and before
 * the first datagram is read.  It may not be called once the first datagram
 * has been read.
 */
bool DatagramInputFile::
read_header(std::string &header, size_t num_bytes) {
  nassertr(!_read_first_datagram, false);
  nassertr(_in != nullptr, false);

  char *buffer = (char *)alloca(num_bytes);
  nassertr(buffer != nullptr, false);

  _in->read(buffer, num_bytes);
  if (_in->fail()) {
    return false;
  }

  header = std::string(buffer, num_bytes);
  Thread::consider_yield();
  return true;
}

/**
 * Reads the next datagram from the file.  Returns true on success, false if
 * there is an error or end of file.
 */
bool DatagramInputFile::
get_datagram(Datagram &data) {
  nassertr(_in != nullptr, false);
  _read_first_datagram = true;

  // First, get the size of the upcoming datagram.
  StreamReader reader(_in, false);
  uint32_t num_bytes_32 = reader.get_uint32();
  if (_in->fail()) {
    return false;
  }

  if (num_bytes_32 == 0) {
    // A special case for a zero-length datagram: no need to try to read any
    // data.
    data.clear();
    return true;
  }

  if (_in->eof()) {
    return false;
  }

  size_t num_bytes = (size_t)num_bytes_32;
  if (num_bytes_32 == (uint32_t)-1) {
    // Another special case for a value larger than 32 bits.
    uint64_t num_bytes_64 = reader.get_uint64();

    if (_in->fail()) {
      _error = true;
      return false;
    }

    if (num_bytes_64 == 0) {
      data.clear();
      return false;
    }

    if (_in->eof()) {
      return false;
    }

    num_bytes = (size_t)num_bytes_64;

    // Make sure we have a reasonable datagram size for putting into memory.
    if (num_bytes_64 != (uint64_t)num_bytes) {
      _error = true;
      return false;
    }
  }

  // Now, read the datagram itself. We construct an empty datagram, use
  // pad_bytes to make it big enough, and read *directly* into the datagram's
  // internal buffer. Doing this saves us a copy operation.
  data = Datagram();

  size_t bytes_read = 0;
  while (bytes_read < num_bytes) {
    size_t bytes_left = num_bytes - bytes_read;

    if (_in->eof()) {
      _error = true;
      return false;
    }

    // Hold up a second - datagrams >4MB are pretty large by bam/network
    // standards. Let's take it 4MB at a time just in case the length is
    // corrupt, so we don't allocate potentially a few GBs of RAM only to
    // find a truncated file.
    bytes_left = std::min(bytes_left, (size_t)4*1024*1024);

    PTA_uchar buffer = data.modify_array();
    buffer.resize(buffer.size() + bytes_left);
    unsigned char *ptr = &buffer.p()[bytes_read];

    _in->read((char *)ptr, (streamsize)bytes_left);
    if (_in->fail()) {
      _error = true;
      return false;
    }

    bytes_read += bytes_left;
  }

  Thread::consider_yield();

  return true;
}

/**
 * Skips over the next datagram without extracting it, but saves the relevant
 * file information in the SubfileInfo object so that its data may be read
 * later.  For non-file-based datagram generators, this may mean creating a
 * temporary file and copying the contents of the datagram to disk.
 *
 * Returns true on success, false on failure or if this method is
 * unimplemented.
 */
bool DatagramInputFile::
save_datagram(SubfileInfo &info) {
  nassertr(_in != nullptr, false);
  _read_first_datagram = true;

  // First, get the size of the upcoming datagram.
  StreamReader reader(_in, false);
  size_t num_bytes_32 = reader.get_uint32();
  if (_in->fail() || (_in->eof() && num_bytes_32 > 0)) {
    return false;
  }

  streamsize num_bytes = (streamsize)num_bytes_32;
  if (num_bytes_32 == (uint32_t)-1) {
    // Another special case for a value larger than 32 bits.
    num_bytes = reader.get_uint64();
  }

  // If this stream is file-based, we can just point the SubfileInfo directly
  // into this file.
  if (_file != nullptr) {
    info = SubfileInfo(_file, _in->tellg(), num_bytes);
    _in->seekg(num_bytes, std::ios::cur);
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

  _in->read(buffer, std::min((streamsize)buffer_size, num_remaining));
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
    _in->read(buffer, std::min((streamsize)buffer_size, num_remaining));
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

/**
 * Returns true if the file has reached the end-of-file.  This test may only
 * be made after a call to read_header() or get_datagram() has failed.
 */
bool DatagramInputFile::
is_eof() {
  return _in != nullptr ? _in->eof() : true;
}

/**
 * Returns true if the file has reached an error condition.
 */
bool DatagramInputFile::
is_error() {
  if (_in == nullptr) {
    return true;
  }

  if (_in->fail()) {
    _error = true;
  }
  return _error;
}

/**
 * Returns the filename that provides the source for these datagrams, if any,
 * or empty string if the datagrams do not originate from a file on disk.
 */
const Filename &DatagramInputFile::
get_filename() {
  return _filename;
}

/**
 * Returns the on-disk timestamp of the file that was read, at the time it was
 * opened, if that is available, or 0 if it is not.
 */
time_t DatagramInputFile::
get_timestamp() const {
  return _timestamp;
}

/**
 * Returns the FileReference that provides the source for these datagrams, if
 * any, or NULL if the datagrams do not originate from a file on disk.
 */
const FileReference *DatagramInputFile::
get_file() {
  return _file;
}

/**
 * Returns the VirtualFile that provides the source for these datagrams, if
 * any, or NULL if the datagrams do not originate from a VirtualFile.
 */
VirtualFile *DatagramInputFile::
get_vfile() {
  return _vfile;
}

/**
 * Returns the current file position within the data stream, if any, or 0 if
 * the file position is not meaningful or cannot be determined.
 *
 * For DatagramInputFiles that return a meaningful file position, this will be
 * pointing to the first byte following the datagram returned after a call to
 * get_datagram().
 */
streampos DatagramInputFile::
get_file_pos() {
  if (_in == nullptr) {
    return 0;
  }
  return _in->tellg();
}
