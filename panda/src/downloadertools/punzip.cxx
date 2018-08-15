/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file punzip.cxx
 */

#include "filename.h"
#include "compress_string.h"
#include "pnotify.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"

using std::cerr;
using std::cin;
using std::cout;
using std::endl;

void
usage() {
  cerr
    << "\nUsage:\n"
    << "   punzip file.pz [file2.pz file3.pz ...]\n"
    << "   punzip -c <file >dest_file\n"
    << "   punzip -o dest_file file.pz\n\n"

    << "This program reverses the operation of a previous pzip command.  It\n"
    << "uncompresses the contents of the named source file(s) and removes the .pz\n"
    << "extension.\n\n";
}

int
main(int argc, char **argv) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "o:ch";

  Filename dest_filename;
  bool got_dest_filename = false;
  bool use_stdout = false;

  preprocess_argv(argc, argv);
  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'o':
      dest_filename = Filename::from_os_specific(optarg);
      got_dest_filename = true;
      break;

    case 'c':
      use_stdout = true;
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

  if (use_stdout) {
    if (argc > 1) {
      cerr << "No filenames allowed in conjunction with -c.\n";
      return 1;
    }

    bool success = decompress_stream(cin, cout);
    if (!success) {
      cerr << "Failure compressing standard input\n";
      return 1;
    }
    return 0;
  }

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
    if (!got_dest_filename && source_file.get_extension() != "pz") {
      cerr << source_file
           << " doesn't end in .pz; can't derive filename of output file.\n";
      all_ok = false;

    } else {
      Filename dest_file = dest_filename;
      if (!got_dest_filename) {
        dest_file = source_file.get_fullpath_wo_extension();
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
          bool success = decompress_stream(read_stream, write_stream);

          read_stream.close();
          write_stream.close();

          if (!success) {
            cerr << "Failure decompressing " << source_file << "\n";
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
