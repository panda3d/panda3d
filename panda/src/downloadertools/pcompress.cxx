// Filename: pcompress.cxx
// Created by:  
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

#include <filename.h>
#include <zcompressor.h>
#include <notify.h>

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: pcompress <file>" << endl;
    return 0;
  }

  Filename source_file = argv[1];
  string dname = argv[1];
  dname += ".pz";
  Filename dest_file = dname;

  // Open source file
  ifstream read_stream;
  source_file.set_binary();
  if (!source_file.open_read(read_stream)) {
    cerr << "failed to open: " << source_file << endl;
    return 0;
  }

  // Determine source file length
  read_stream.seekg(0, ios::end);
  int source_file_length = read_stream.tellg();
  read_stream.seekg(0, ios::beg);

  if (source_file_length == 0) {
    cerr << "zero length file: " << source_file << endl;
    return 1;
  }

  // Open destination file
  ofstream write_stream;
  dest_file.set_binary();
  if (!dest_file.open_write(write_stream)) {
    cerr << "failed to open: " << dest_file << endl;
    return 0;
  }

  ZCompressor compressor;
  int buffer_length = 1000000;
  int half_buffer_length = buffer_length/2;
  char *buffer = new char[buffer_length];

  // Read from the source file into the first half of the buffer,
  // decompress into the second half of the buffer, write the second
  // half of the buffer to disk, and repeat.
  int total_bytes_read = 0;
  bool read_all_input = false;
  bool handled_all_input = false;
  int source_buffer_length;
  int ret;
  while (handled_all_input == false) {

    // See if there is anything left in the source file
    if (read_all_input == false) {
      read_stream.read(buffer, half_buffer_length);
      source_buffer_length = read_stream.gcount();
      total_bytes_read += source_buffer_length;
      if (read_stream.eof()) {
        nassertr(total_bytes_read == source_file_length, false);
        read_all_input = true;
      }
    }

    char *next_in = buffer;
    int avail_in = source_buffer_length;
    char *dest_buffer = buffer + source_buffer_length;
    char *next_out = dest_buffer;
    int dest_buffer_length = buffer_length - source_buffer_length;
    int avail_out = dest_buffer_length;
    nassertr(avail_out > 0 && avail_in > 0, false);

    // Consume all the input from the input buffer, writing to disk
    // as we go to free output buffer space
    while (avail_in > 0) {
      ret = compressor.compress_to_stream(next_in, avail_in,
                      next_out, avail_out, dest_buffer,
                      dest_buffer_length, write_stream);
      if (ret == ZCompressorBase::S_error)
        return 0;
    }

    // If all the input has been consumed, we need to keep processing
    // output until it says it's finished
    if (compressor.get_total_in() == source_file_length) {
      while (ret != ZCompressorBase::S_finished) {
        ret = compressor.compress_to_stream(next_in, avail_in,
                      next_out, avail_out, dest_buffer,
                      dest_buffer_length, write_stream, true);
        if (ret == ZCompressorBase::S_error)
          return 0;
      }
      handled_all_input = true;
    }
  }

  read_stream.close();
  write_stream.close();
  source_file.unlink();

  return 1;
}
