// Filename: newheader.cxx
// Created by:  drose (05Jul04)
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

#include "dtoolbase.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>

char *cxx_style = 
"// Filename: %s\n"
"// Created by:  %s (%s)\n"
"//\n"
"////////////////////////////////////////////////////////////////////\n"
"//\n"
"// PANDA 3D SOFTWARE\n"
"// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved\n"
"//\n"
"// All use of this software is subject to the terms of the Panda 3d\n"
"// Software license.  You should have received a copy of this license\n"
"// along with this source code; you will also find a current copy of\n"
"// the license at http://etc.cmu.edu/panda3d/docs/license/ .\n"
"//\n"
"// To contact the maintainers of this program write to\n"
"// panda3d-general@lists.sourceforge.net .\n"
"//\n"
"////////////////////////////////////////////////////////////////////\n"
"\n";

char *c_style = 
"/* Filename: %s\n"
" * Created by:  %s (%s)\n"
" *\n"
" * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\n"
" *\n"
" * PANDA 3D SOFTWARE\n"
" * Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved\n"
" *\n"
" * All use of this software is subject to the terms of the Panda 3d\n"
" * Software license.  You should have received a copy of this license\n"
" * along with this source code; you will also find a current copy of\n"
" * the license at http://etc.cmu.edu/panda3d/docs/license/ .\n"
" *\n"
" * To contact the maintainers of this program write to\n"
" * panda3d-general@lists.sourceforge.net .\n"
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
  char *username = getenv("USER");
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
