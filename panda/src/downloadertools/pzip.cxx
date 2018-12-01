/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pzip.cxx
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
    << "   pzip file [file2 file3 ...]\n"
    << "   pzip -c <file >dest_file\n"
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
    << "multify has an inline compression option.\n\n"

    << "Options:\n\n"

    << "  -1  compress faster\n"
    << "  -6  compress default\n"
    << "  -9  compress better (intermediate compression levels supported also)\n\n";

}

int
main(int argc, char **argv) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "o:c123456789h";

  Filename dest_filename;
  bool got_dest_filename = false;
  bool use_stdout = false;
  int compression_level = 6;

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

    case '1':
      compression_level = 1;
      break;

    case '2':
      compression_level = 2;
      break;

    case '3':
      compression_level = 3;
      break;

    case '4':
      compression_level = 4;
      break;

    case '5':
      compression_level = 5;
      break;

    case '6':
      compression_level = 6;
      break;

    case '7':
      compression_level = 7;
      break;

    case '8':
      compression_level = 8;
      break;

    case '9':
      compression_level = 9;
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

    bool success = compress_stream(cin, cout, compression_level);
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
          bool success = compress_stream(read_stream, write_stream, compression_level);

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
