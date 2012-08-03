// Filename: check_md5.cxx
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

#include "pandabase.h"
#include "pystub.h"
#include "hashVal.h"
#include "filename.h"
#include "panda_getopt.h"
#include "preprocess_argv.h"

bool output_decimal = false;
bool suppress_filename = false;
pofstream binary_output;

void
usage() {
  cerr << 
    "\n"
    "Usage:\n\n"
    "check_md5 [-q] [-d] [-b filename] [-i \"input string\"] [file1 file2 ...]\n"
    "check_md5 -h\n\n";
}

void
help() {
  usage();
  cerr << 
    "This program outputs the MD5 hash of one or more files (or of a string\n"
    "passed on the command line with -i).\n\n"

    "An MD5 hash is a 128-bit value.  The output is presented as a 32-digit\n"
    "hexadecimal string by default, but with -d, it is presented as four\n"
    "big-endian unsigned 32-bit decimal integers.  Normally the filename\n"
    "of each file is printed along with the hash; -q suppresses this.\n\n"
    
    "To write the 16 bytes (per input file) of the output directly to a\n"
    "binary file, use -b with the name of the file to receive the output.\n";
}

void
output_hash(const string &filename, const HashVal &hash) {
  if (!suppress_filename && !filename.empty()) {
    cout << filename << " ";
  }
  if (output_decimal) {
    hash.output_dec(cout);
  } else {
    hash.output_hex(cout);
  }
  cout << "\n";

  // Also output to the binary_output file if it is open.  No sweat if
  // it's not.
  hash.output_binary(binary_output);
}
  

int
main(int argc, char **argv) {
  // A call to pystub() to force libpystub.so to be linked in.
  pystub();

  extern char *optarg;
  extern int optind;
  const char *optstr = "i:db:qh";

  bool got_input_string = false;
  string input_string;
  Filename binary_output_filename;

  preprocess_argv(argc, argv);
  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
    case 'i':
      got_input_string = true;
      input_string = optarg;
      break;

    case 'd':
      output_decimal = true;
      break;

    case 'b':
      binary_output_filename = Filename::binary_filename(string(optarg));
      break;

    case 'q':
      suppress_filename = true;
      break;

    case 'h':
      help();
      exit(1);

    default:
      exit(1);
    }
    flag = getopt(argc, argv, optstr);
  }

  argc -= (optind-1);
  argv += (optind-1);

  if (argc < 2 && !got_input_string) {
    usage();
    exit(1);
  }

  if (!binary_output_filename.empty()) {
    if (!binary_output_filename.open_write(binary_output)) {
      cerr << "Unable to open " << binary_output_filename << ".\n";
      exit(1);
    }
  }

  if (got_input_string) {
    HashVal hash;
    hash.hash_string(input_string);
    output_hash("", hash);
  }

  bool okflag = true;

  for (int i = 1; i < argc; i++) {
    Filename source_file = Filename::from_os_specific(argv[i]);

    if (!source_file.exists()) {
      cerr << source_file << " not found!\n";
      okflag = false;
    } else {
      HashVal hash;
      if (!hash.hash_file(source_file)) {
        cerr << "Unable to read " << source_file << "\n";
        okflag = false;
      } else {
        output_hash(source_file.get_basename(), hash);
      }
    }
  }

  if (!okflag) {
    exit(1);
  }

  return 0;
}
