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

#include "filename.h"
#include "zStream.h"
#include "notify.h"

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Usage: pcompress <file>" << endl;
    return 1;
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
  if (!dest_file.open_write(write_stream)) {
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
  source_file.unlink();

  return 0;
}
