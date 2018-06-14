/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pencrypt.cxx
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
std::string algorithm;
bool got_algorithm = false;
int key_length = -1;
bool got_key_length = false;
int iteration_count = -1;
bool got_iteration_count = false;

void
usage() {
  cerr
    << "\nUsage:\n"
    << "   pencrypt [opts] file [file2 file3 ...]\n"
    << "   pencrypt -o dest_file file\n\n"

    << "This program will apply an encryption algorithm to a file (or multiple files),\n"
    << "creating an encrypted version of each file which can only be recovered using\n"
    << "pdecrypt and the same password that was supplied to pencrypt.  The compressed\n"
    << "versions are written to a file with the same name as the original, but the\n"
    << "extension .pe added to the filename, and the original file is removed\n"
    << "(unless the version with -o is used, in which case you can encrypt only one\n"
    << "file, you specify the destination file name, and the original file is not\n"
    << "removed).\n\n"


    << "Note that if you are adding files to a Panda multifile (.mf file) with\n"
    << "the multify command, it is not necessary to encrypt them separately;\n"
    << "multify has an inline encryption option.\n\n"

    << "Options:\n\n"

    << "  -p \"password\"\n"
    << "      Specifies the password to use for encryption.  There are no\n"
    << "      restrictions on the password length or contents, but longer passwords\n"
    << "      are more secure.  If this is not specified, the user is prompted from\n"
    << "      standard input.\n\n"

    << "  -t  Read the file as a text file.  This will automatically convert\n"
    << "      universal end-of-line characters into a newline character, ascii\n"
    << "      10.\n\n"

    << "  -a \"algorithm\"\n"
    << "      Specifies the particular encryption algorithm to use.  The complete\n"
    << "      set of available algorithms is defined by the current version of\n"
    << "      OpenSSL.  The default algorithm is taken from the encryption-\n"
    << "      algorithm config variable.\n\n"

    << "  -k key_length\n"
    << "      Specifies the key length, in bits, for the selected encryption\n"
    << "      algorithm.  This only makes sense for those algorithms that support\n"
    << "      a variable key length.  The default value is taken from the\n"
    << "      encryption-key-length config variable.\n\n"

    << "  -i iteration_count\n"
    << "      Specifies the number of times the password is hashed to generate\n"
    << "      a key.  The only purpose of this is to make it computationally\n"
    << "      more expensive for an attacker to search the key space exhaustively.\n"
    << "      This should be a multiple of 1,000 and should not exceed about 65\n"
    << "      million; the value 0 indicates just one application of the hashing\n"
    << "      algorithm.  The default value is taken from the encryption-iteration-\n"
    << "      count config variable.\n\n";
}

int
main(int argc, char **argv) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "o:p:ta:k:i:h";

  Filename dest_filename;
  bool got_dest_filename = false;
  bool text_file = false;

  preprocess_argv(argc, argv);
  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'o':
      dest_filename = Filename::from_os_specific(optarg);
      got_dest_filename = true;
      break;

    case 't':
      text_file = true;
      break;

    case 'p':
      password = optarg;
      got_password = true;
      break;

    case 'a':
      algorithm = optarg;
      got_algorithm = true;
      break;

    case 'k':
      key_length = atoi(optarg);
      got_key_length = true;
      break;

    case 'i':
      iteration_count = atoi(optarg);
      got_iteration_count = true;
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
    if (source_file.get_extension() == "pe") {
      cerr << source_file << " already ends .pe; skipping.\n";
    } else {
      Filename dest_file = dest_filename;
      if (!got_dest_filename) {
        dest_file = source_file.get_fullpath() + ".pe";
      }

      // Open source file
      pifstream read_stream;
      if (text_file) {
        source_file.set_text();
      } else {
        source_file.set_binary();
      }
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
          bool success = encrypt_stream(read_stream, write_stream, password,
                                        algorithm, key_length, iteration_count);

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
