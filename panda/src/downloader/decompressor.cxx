// Filename: decompressor.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

// This file is compiled only if we have zlib installed.

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include "config_downloader.h"

#include <stdio.h>
#include <errno.h>

#include <error_utils.h>
#include <filename.h>

#include "decompressor.h"


////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
Decompressor(void) {
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Decompressor::constructor() - Creating buffer of size: "
      << decompressor_buffer_size << endl;
  PT(Buffer) buffer = new Buffer(decompressor_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
Decompressor(PT(Buffer) buffer) {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Decompressor::
init(PT(Buffer) buffer) {
  _initiated = false;
  nassertv(!buffer.is_null());
  _half_buffer_length = buffer->get_length()/2;
  _buffer = buffer;
  char *temp_name = tempnam(NULL, "dc");
  _temp_file_name = temp_name;
  _temp_file_name.set_binary();
  delete temp_name;
  _decompressor = new ZDecompressor();
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
~Decompressor(void) {
  _temp_file_name.unlink();
  if (_initiated == true)
    cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::initiate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Decompressor::
initiate(Filename &source_file) {
  Filename dest_file = source_file;
  string extension = source_file.get_extension();
  if (extension == "pz")
    dest_file = source_file.get_fullpath_wo_extension();
  else {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Decompressor::request_decompress() - Unknown file extension: ."
        << extension << endl;
      return EU_error_abort;
  }
  return initiate(source_file, dest_file);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::initiate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Decompressor::
initiate(Filename &source_file, Filename &dest_file) {

  if (_initiated == true) {
    downloader_cat.error()
      << "Decompressor::initiate() - Decompression has already been initiated"
      << endl;
    return EU_error_abort;
  }

  // Open source file
  _source_file = source_file;
  _source_file.set_binary();
  if (!_source_file.open_read(_read_stream)) {
    downloader_cat.error()
      << "Decompressor::initiate() - Error opening source file: "
      << _source_file << " : " << strerror(errno) << endl;
    return get_write_error();
  }

  // Determine source file length
  _read_stream.seekg(0, ios::end);
  _source_file_length = _read_stream.tellg();
  if (_source_file_length == 0) {
    downloader_cat.warning()
      << "Decompressor::initiate() - Zero length file: "
      << source_file << " : " << strerror(errno) << endl;
    return get_write_error();
  }
  _read_stream.seekg(0, ios::beg);

  // Open destination file
  dest_file.set_binary();
  if (!dest_file.open_write(_write_stream)) {
    downloader_cat.error()
      << "Decompressor::initiate() - Error opening dest file: "
      << source_file << " : " << strerror(errno) << endl;
    return get_write_error();
  }

  // Read from the source file into the first half of the buffer,
  // decompress into the second half of the buffer, write the second
  // half of the buffer to disk, and repeat.
  _total_bytes_read = 0;
  _read_all_input = false;
  _source_buffer_length = 0;
  _initiated = true;
  _decompressor = new ZDecompressor();
  _decompress_to_ram = false;
  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::initiate
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Decompressor::
initiate(Ramfile &source_file) {

  if (_initiated == true) {
    downloader_cat.error()
      << "Decompressor::initiate() - Decompression has already been initiated"
      << endl;
    return EU_error_abort;
  }

  // Open source file
  _read_string_stream = new istringstream(source_file._data);

  // Determine source file length
  _source_file_length = source_file._data.length();
  if (_source_file_length == 0) {
    downloader_cat.warning()
      << "Decompressor::initiate() - Zero length file: "
      << strerror(errno) << endl;
    return get_write_error();
  }

  // Open destination file
  _write_string_stream = new ostringstream();

  // Read from the source file into the first half of the buffer,
  // decompress into the second half of the buffer, write the second
  // half of the buffer to disk, and repeat.
  _total_bytes_read = 0;
  _read_all_input = false;
  _source_buffer_length = 0;
  _initiated = true;
  _decompressor = new ZDecompressor();
  _decompress_to_ram = true;
  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::cleanup
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Decompressor::
cleanup(void) {
  if (_initiated == false) {
    downloader_cat.error()
      << "Decompressor::cleanup() - Decompression has not been "
      << "initiated" << endl;
    return;
  }

  _initiated = false;
  delete _decompressor;
  _decompressor = NULL;
  _read_stream.close();
  _write_stream.close();
  if (_decompress_to_ram == false)
    _source_file.unlink();
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Decompressor::
run(void) {
  if (_initiated == false) {
    downloader_cat.error()
      << "Decompressor::run() - Decompression has not been initiated"
      << endl;
    return EU_error_abort;
  }

  // See if there is anything left in the source file
  if (_read_all_input == false) {
    if (_decompress_to_ram == false) {
      _read_stream.read(_buffer->_buffer, _half_buffer_length);
      _source_buffer_length = _read_stream.gcount();
      _total_bytes_read += _source_buffer_length;
      if (_read_stream.eof()) {
        nassertr(_total_bytes_read == _source_file_length, false);
        _read_all_input = true;
      }
    } else {
      _read_string_stream->read(_buffer->_buffer, _half_buffer_length);
      _source_buffer_length = _read_string_stream->gcount();
      _total_bytes_read += _source_buffer_length;
      if (_read_string_stream->eof()) {
        nassertr(_total_bytes_read == _source_file_length, false);
        _read_all_input = true;
      }
    }
  }

  char *next_in = _buffer->_buffer;
  int avail_in = _source_buffer_length;
  char *dest_buffer = _buffer->_buffer + _source_buffer_length;
  char *next_out = dest_buffer;
  int dest_buffer_length = _buffer->get_length() - _source_buffer_length;
  int avail_out = dest_buffer_length;
  nassertr(avail_out > 0 && avail_in > 0, false);

  while (avail_in > 0) {
    int ret;
    if (_decompress_to_ram == false)
      ret = _decompressor->decompress_to_stream(next_in, avail_in,
                        next_out, avail_out, dest_buffer,
                        dest_buffer_length, _write_stream);
    else
      ret = _decompressor->decompress_to_stream(next_in, avail_in,
                        next_out, avail_out, dest_buffer,
                        dest_buffer_length, *_write_string_stream);
    if (ret == ZCompressorBase::S_error)
      return EU_error_zlib;
    if ((int)_decompressor->get_total_in() == _source_file_length &&
          avail_out == dest_buffer_length) {
      cleanup();
      return EU_success;
    }
  }

  return EU_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::decompress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Decompressor::
decompress(Filename &source_file) {
  int ret = initiate(source_file);
  if (ret < 0)
    return false;
  for (;;) {
    ret = run();
    if (ret == EU_success)
      return true;
    else if (ret < 0)
      return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::decompress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Decompressor::
decompress(Ramfile &source_file) {
  int ret = initiate(source_file);
  if (ret < 0)
    return false;
  for (;;) {
    ret = run();
    if (ret == EU_success) {
      source_file._data = _write_string_stream->str();
      delete _read_string_stream;
      _read_string_stream = NULL;
      delete _write_string_stream;
      _write_string_stream = NULL;
      return true;
    } else if (ret < 0)
      return false;
  }
  return false;
}
