// Filename: pzip.cxx
// Created by:  
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

#include "filename.h"
#include "zStream.h"
#include "pnotify.h"

#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #ifdef HAVE_GETOPT_H
    #include <getopt.h>
  #endif
#endif

bool
do_compress(istream &read_stream, ostream &write_stream) {
  OCompressStream compress(&write_stream, false);

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  read_stream.read(buffer, buffer_size);
  size_t count = read_stream.gcount();
  while (count != 0) {
    compress.write(buffer, count);
    read_stream.read(buffer, buffer_size);
    count = read_stream.gcount();
  }
  compress.close();

  return !read_stream.fail() || read_stream.eof() &&
    (!compress.fail() || compress.eof());
}

void
usage() {
  cerr
    << "\nUsage:\n"
    << "   pzip file [file2 file3 ...]\n"
    << "   pzip -o dest_file file\n\n"
    
    << "This program compresses the named file(s) using the Panda native\n"
    << "compression algorithm (gzip in practice, but with a different file\n"
    << "header).  The compressed versions are written to a file with the\n"
    << "same name as the original, but the extension .pz added to the\n"
    << "filename, and the original file is removed (unless the version with\n"
    << "-o is used, in which case you can compress only one file, you specify\n"
    << "the destination file name, and the original file is not removed).\n\n"
    
    << "In many cases, Panda can read the resulting .pz file directly,\n"
    << "exactly as if it were still in its uncompressed original form.\n"
    << "In fact, unless vfs-implicit-pz is set to false in your Config.prc\n"
    << "file, you can also load the file by referencing it with its original\n"
    << "filename (without the .pz extension), even though it no longer exists\n"
    << "under that filename, and Panda will find the .pz file and transparently\n"
    << "decompress it on the fly, as if the original, uncompressed file still\n"
    << "existed.\n\n"

    << "Note that if you are adding files to a Panda multifile (.mf file) with\n"
    << "the multify command, it is not necessary to compress them separately;\n"
    << "multify has an inline compression option.\n\n";
}

int
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "o:h";

  Filename dest_filename;
  bool got_dest_filename = false;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'o':
      dest_filename = Filename::from_os_specific(optarg);
      got_dest_filename = true;
      break;

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

  if (got_dest_filename && argc > 2) {
    cerr << "Only one input file allowed in conjunction with -o.\n";
    return 1;
  }

  bool all_ok = true;
  for (int i = 1; i < argc; i++) {
    Filename source_file = Filename::from_os_specific(argv[i]);
    if (source_file.get_extension() == "pz") {
      cerr << source_file << " already ends .pz; skipping.\n";
    } else {
      Filename dest_file = dest_filename;
      if (!got_dest_filename) {
        dest_file = source_file.get_fullpath() + ".pz";
      }

      // Open source file
      pifstream read_stream;
      source_file.set_binary();
      if (!source_file.open_read(read_stream)) {
        cerr << "Couldn't read: " << source_file << endl;
        all_ok = false;

      } else {
        // Open destination file
        pofstream write_stream;
        dest_file.set_binary();
        if (!dest_file.open_write(write_stream, true)) {
          cerr << "Failed to open: " << dest_file << endl;
          all_ok = false;

        } else {
          cerr << dest_file << "\n";
          bool success = do_compress(read_stream, write_stream);
          
          read_stream.close();
          write_stream.close();
          
          if (!success) {
            cerr << "Failure writing " << dest_file << "\n";
            all_ok = false;
            dest_file.unlink();
            
          } else {
            if (!got_dest_filename) {
              source_file.unlink();
            }
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
