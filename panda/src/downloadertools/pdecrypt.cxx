// Filename: pdecrypt.cxx
// Created by:  drose (01Sep04)
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
#include "encryptStream.h"
#include "notify.h"

#ifndef HAVE_GETOPT
  #include "gnu_getopt.h"
#else
  #ifdef HAVE_GETOPT_H
    #include <getopt.h>
  #endif
#endif

void 
usage() {
  cerr
    << "\n"
    << "Usage: pdecrypt [opts] <file> [<dest_file>]\n\n"
    
    << "This program reverses the operation of a previous pencrypt command.  It\n"
    << "decrypts the contents of the source file by applying the indicated password.\n"
    << "The encryption algorithm need not be specified; it can be determined by\n"
    << "examining the header of the encrypted file.  The password must match exactly.\n"
    << "If it does not, an error may or may not be reported; but the file will not be\n"
    << "decrypted correctly even if no error is reported.\n\n"

    << "Options:\n\n"
    
    << "  -p \"password\"\n"
    << "      Specifies the password to use for descryption.  If this is not specified,\n"
    << "      the user is prompted from standard input.\n\n";
}

int
main(int argc, char *argv[]) {
  extern char *optarg;
  extern int optind;
  const char *optstr = "p:h";

  string password;
  bool got_password = false;

  int flag = getopt(argc, argv, optstr);

  while (flag != EOF) {
    switch (flag) {
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

  bool implicit_dest_file;
  Filename source_file = Filename::from_os_specific(argv[1]);
  Filename dest_file;
  if (argc < 3) {
    if (source_file.get_extension() == "pe") {
      dest_file = source_file;
      dest_file.set_extension("");
    } else {
      cerr << "Input filename doesn't end in .pe; can't derive filename of output file.\n";
      return 1;
    }
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

  // Open destination file
  ofstream write_stream;
  dest_file.set_binary();
  if (!dest_file.open_write(write_stream, true)) {
    cerr << "failed to open: " << dest_file << endl;
    return 1;
  }

  // Prompt for password.
  if (!got_password) {
    cerr << "Enter password: ";
    getline(cin, password);
  }

  bool fail = false;
  {
    IDecryptStream decrypt(&read_stream, false, password);
    
    int ch = decrypt.get();
    while (!decrypt.eof() && !decrypt.fail()) {
      write_stream.put(ch);
      ch = decrypt.get();
    }

    fail = decrypt.fail() && !decrypt.eof();
  }

  read_stream.close();
  write_stream.close();

  if (fail) {
    dest_file.unlink();

  } else {
    if (implicit_dest_file) {
      source_file.unlink();
    }
  }

  return 0;
}
