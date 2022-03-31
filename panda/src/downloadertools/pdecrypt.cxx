/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pdecrypt.cxx
 * @author drose
 * @date 2004-09-01
 */

#include "filename.h"
#include "encrypt_string.h"
#include "pnotify.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"

using std::cerr;
using std::endl;

std::string password;
bool got_password = false;

void
usage() {
  cerr
    << "\nUsage:\n"
    << "   pdecrypt file.pe [file2.pe file3.pe ...]\n"
    << "   pdecrypt -o dest_file file.pe\n\n"
    << "\n"

    << "This program reverses the operation of a previous pencrypt command.  It\n"
    << "decrypts the contents of the named source file(s) and removes the .pe\n"
    << "extension.  The encryption algorithm need not be specified; it can be\n"
    << "determined by examining the header of each encrypted file.  The password\n"
    << "must match the encryption password exactly.  If it does not, an error may\n"
    << "or may not be reported; but the file will not be decrypted correctly even\n"
    << "if no error is reported.\n\n"

    << "Options:\n\n"

    << "  -p \"password\"\n"
    << "      Specifies the password to use for decryption.  If this is not specified,\n"
    << "      the user is prompted from standard input.\n\n";
}

int
main(int argc, char **argv) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "o:p:h";

  Filename dest_filename;
  bool got_dest_filename = false;

  preprocess_argv(argc, argv);
  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'o':
      dest_filename = Filename::from_os_specific(optarg);
      got_dest_filename = true;
      break;

    case 'p':
      password = optarg;
      got_password = true;
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
    if (!got_dest_filename && source_file.get_extension() != "pe") {
      cerr << source_file
           << " doesn't end in .pe; can't derive filename of output file.\n";
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
          // Prompt for password.
          if (!got_password) {
            cerr << "Enter password: ";
            std::getline(std::cin, password);
            got_password = true;
          }

          cerr << dest_file << "\n";
          bool success = decrypt_stream(read_stream, write_stream, password);

          read_stream.close();
          write_stream.close();

          if (!success) {
            cerr << "Failure decrypting " << source_file << "\n";
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
