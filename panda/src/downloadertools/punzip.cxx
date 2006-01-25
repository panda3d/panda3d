// Filename: punzip.cxx
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

#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #ifdef HAVE_GETOPT_H
    #include <getopt.h>
  #endif
#endif

bool
do_decompress(istream &read_stream, ostream &write_stream) {
  IDecompressStream decompress(&read_stream, false);

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  decompress.read(buffer, buffer_size);
  size_t count = decompress.gcount();
  while (count != 0) {
    write_stream.write(buffer, count);
    decompress.read(buffer, buffer_size);
    count = decompress.gcount();
  }
  
  return !decompress.fail() || decompress.eof() &&
    (!write_stream.fail() || write_stream.eof());
}

void
usage() {
  cerr
    << "\nUsage: punzip file.pz [file2.pz file3.pz ...]\n\n"
    
    << "This program reverses the operation of a previous pzip command.  It\n"
    << "uncompresses the contents of the named source file(s) and removes the .pz\n"
    << "extension.\n\n";
}

int
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "h";

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'h':
    case '?':
    default:
      usage();
      return 1;
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2) {
    usage();
    return 1;
  }

  bool all_ok = true;
  for (int i = 1; i < argc; i++) {
    Filename source_file = Filename::from_os_specific(argv[i]);
    if (source_file.get_extension() != "pz") {
      cerr << source_file 
           << " doesn't end in .pz; can't derive filename of output file.\n";
      all_ok = false;

    } else {
      Filename dest_file = source_file.get_fullpath_wo_extension();

      // Open source file
      ifstream read_stream;
      source_file.set_binary();
      if (!source_file.open_read(read_stream)) {
        cerr << "Couldn't read: " << source_file << endl;
        all_ok = false;

      } else {
        // Open destination file
        ofstream write_stream;
        dest_file.set_binary();
        if (!dest_file.open_write(write_stream, true)) {
          cerr << "Failed to open: " << dest_file << endl;
          all_ok = false;

        } else {
          cerr << dest_file << "\n";
          bool success = do_decompress(read_stream, write_stream);
          
          read_stream.close();
          write_stream.close();
          
          if (!success) {
            cerr << "Failure decompressing " << source_file << "\n";
            all_ok = false;
            dest_file.unlink();
            
          } else {
            source_file.unlink();
          }
        }
      }
    }
  }

  if (all_ok) {
    return 0;
  } else {
    return 1;
  }
}
