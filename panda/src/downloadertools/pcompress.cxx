// Filename: pcompress.cxx
// Created by:  
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

#include "filename.h"
#include "zStream.h"
#include "notify.h"

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: pcompress <file> [<dest_file>]" << endl;
    return 1;
  }

  bool implicit_dest_file;
  Filename source_file = Filename::from_os_specific(argv[1]);
  Filename dest_file;
  if (argc < 3) {
    dest_file = source_file.get_fullpath() + ".pz";
    implicit_dest_file = true;
  } else {
    dest_file = Filename::from_os_specific(argv[2]);
    implicit_dest_file = false;
  }

  // Open source file
  ifstream read_stream;
  source_file.set_binary();
  if (!source_file.open_read(read_stream)) {
    cerr << "failed to open: " << source_file << endl;
    return 1;
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
  if (!dest_file.open_write(write_stream, true)) {
    cerr << "failed to open: " << dest_file << endl;
    return 1;
  }

  {
    OCompressStream compress(&write_stream, false);
    
    int ch = read_stream.get();
    while (!read_stream.eof() && !read_stream.fail()) {
      compress.put(ch);
      ch = read_stream.get();
    }
  }

  read_stream.close();
  write_stream.close();

  if (implicit_dest_file) {
    source_file.unlink();
  }

  return 0;
}
