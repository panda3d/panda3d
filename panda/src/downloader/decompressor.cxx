// Filename: decompressor.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

// This file is compiled only if we have zlib installed.

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "decompressor.h"
#include "config_downloader.h"

#include <filename.h>
#include <stdio.h>

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
  nassertv(!buffer.is_null());
  _half_buffer_length = buffer->get_length()/2; 
  _buffer = buffer;
  char *temp_name = tempnam(NULL, "dc");
  _temp_file_name = temp_name;
  _temp_file_name.set_binary();
  delete temp_name;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
~Decompressor(void) {
  _temp_file_name.unlink();
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

  // Open source file
  _source_file = source_file;
  _source_file.set_binary();
  if (!_source_file.open_read(_read_stream)) {
    downloader_cat.error()
      << "Decompressor::decompress() - Error opening source file: " 
      << _source_file << endl;
    return false;
  } 

  // Determine source file length
  _read_stream.seekg(0, ios::end);
  _source_file_length = _read_stream.tellg();
  if (_source_file_length == 0) {
    downloader_cat.warning()
      << "Decompressor::decompress() - Zero length file: "
      << source_file << endl;
    return true;
  }
  _read_stream.seekg(0, ios::beg);

  // Open destination file 
  dest_file.set_binary();
  if (!dest_file.open_write(_write_stream)) {
    downloader_cat.error()
      << "Decompressor::decompress() - Error opening dest file: " 
      << source_file << endl;
    return false;
  } 

  // Read from the source file into the first half of the buffer,
  // decompress into the second half of the buffer, write the second
  // half of the buffer to disk, and repeat.
  _total_bytes_read = 0;
  _read_all_input = false;
  _source_buffer_length;
  return 1;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Decompressor::
run(void) {

  // See if there is anything left in the source file
  if (_read_all_input == false) {
    _read_stream.read(_buffer->_buffer, _half_buffer_length);
    _source_buffer_length = _read_stream.gcount();
    _total_bytes_read += _source_buffer_length;
    if (_read_stream.eof()) {
      nassertr(_total_bytes_read == _source_file_length, false);
      _read_all_input = true;
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
    int ret = _decompressor.decompress_to_stream(next_in, avail_in,
			next_out, avail_out, dest_buffer, 
			dest_buffer_length, _write_stream);
    if (ret == ZCompressorBase::S_error)
      return DS_error_zlib;
    if ((int)_decompressor.get_total_in() == _source_file_length &&
	  avail_out == dest_buffer_length)
      _read_stream.close();
      _write_stream.close();
      _source_file.unlink();
      return DS_success;
  }

  return DS_ok;
}
