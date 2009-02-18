// Filename: newheader.cxx
// Created by:  drose (05Jul04)
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

#include "dtoolbase.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

const char *cxx_style = 
"// Filename: %s\n"
"// Created by:  %s (%s)\n"
"//\n"
"////////////////////////////////////////////////////////////////////\n"
"//\n"
"// PANDA 3D SOFTWARE\n"
"// Copyright (c) Carnegie Mellon University.  All rights reserved.\n"
"//\n"
"// All use of this software is subject to the terms of the revised BSD\n"
"// license.  You should have received a copy of this license along\n"
"// with this source code in a file named \"LICENSE.\"\n"
"//\n"
"////////////////////////////////////////////////////////////////////\n"
"\n";

const char *c_style = 
"/* Filename: %s\n"
" * Created by:  %s (%s)\n"
" *\n"
" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n"
" *\n"
" * PANDA 3D SOFTWARE\n"
" * Copyright (c) Carnegie Mellon University.  All rights reserved.\n"
" *\n"
" * All use of this software is subject to the terms of the revised BSD\n"
" * license.  You should have received a copy of this license along\n"
" * with this source code in a file named \"LICENSE.\"\n"
" *\n"
" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */\n"
"\n";

struct FileDef {
  const char *extension;
  const char *header;
};

FileDef file_def[] = {
  { "h", cxx_style },
  { "cxx", cxx_style },
  { "I", cxx_style },
  { "T", cxx_style },
  { "c", c_style },
  { NULL, NULL },
};

void
generate_header(const char *header, const string &filename) {
  const char *username = getenv("USER");
  if (username == NULL) {
    username = "";
  }

  static const size_t max_date_buffer = 128;
  char date_buffer[max_date_buffer];
  time_t now = time(NULL);
  strftime(date_buffer, max_date_buffer, "%d%b%y", localtime(&now));

  printf(header, filename.c_str(), username, date_buffer);
}

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Must specify the filename to generate a header for.\n";
    exit(1);
  }

  string filename = argv[1];
  size_t dot = filename.rfind('.');
  if (dot == string::npos) {
    // No extension, no header.
    return 0;
  }

  string extension = filename.substr(dot + 1);

  size_t i = 0;
  while (file_def[i].extension != NULL) {
    if (extension == file_def[i].extension) {
      generate_header(file_def[i].header, filename);
      return 0;
    }
    i++;
  }

  // No matching extension, no problem.
  return 0;
}
