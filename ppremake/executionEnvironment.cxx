// Filename: executionEnvironment.cxx
// Created by:  drose (15May00)
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

#include "executionEnvironment.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <stdio.h>  // for perror

#ifdef WIN32_VC
// Windows requires this for getcwd().
#include <direct.h>
#define getcwd _getcwd
#endif

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnvironment::get_environment_variable
//       Access: Public, Static
//  Description: Returns the definition of the indicated environment
//               variable, or the empty string if the variable is
//               undefined.  The nonstatic implementation.
////////////////////////////////////////////////////////////////////
string ExecutionEnvironment::
get_environment_variable(const string &var) {
  const char *def = getenv(var.c_str());
  if (def != (char *)NULL) {
    return def;
  }
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnviroment::expand_string
//       Access: Public, Static
//  Description: Reads the string, looking for environment variable
//               names marked by a $.  Expands all such variable
//               names.  A repeated dollar sign ($$) is mapped to a
//               single dollar sign.
//
//               Returns the expanded string.
////////////////////////////////////////////////////////////////////
string ExecutionEnvironment::
expand_string(const string &str) {
  string result;

  size_t last = 0;
  size_t dollar = str.find('$');
  while (dollar != string::npos && dollar + 1 < str.length()) {
    size_t start = dollar + 1;

    if (str[start] == '$') {
      // A double dollar sign maps to a single dollar sign.
      result += str.substr(last, start - last);
      last = start + 1;

    } else {
      string varname;
      size_t end = start;

      if (str[start] == '{') {
        // Curly braces delimit the variable name explicitly.
        end = str.find('}', start + 1);
        if (end != string::npos) {
          varname = str.substr(start + 1, end - (start + 1));
          end++;
        }
      }

      if (end == start) {
        // Scan for the end of the variable name.
        while (end < str.length() && (isalnum(str[end]) || str[end] == '_')) {
          end++;
        }
        varname = str.substr(start, end - start);
      }

      string subst =
      result += str.substr(last, dollar - last);
      result += get_environment_variable(varname);
      last = end;
    }

    dollar = str.find('$', last);
  }

  result += str.substr(last);

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: ExecutionEnviroment::get_cwd
//       Access: Public, Static
//  Description: Returns the name of the current working directory.
////////////////////////////////////////////////////////////////////
Filename ExecutionEnvironment::
get_cwd() {
  // getcwd() requires us to allocate a dynamic buffer and grow it on
  // demand.
  static size_t bufsize = 1024;
  static char *buffer = NULL;

  if (buffer == (char *)NULL) {
    buffer = new char[bufsize];
  }

  while (getcwd(buffer, bufsize) == (char *)NULL) {
    if (errno != ERANGE) {
      perror("getcwd");
      return string();
    }
    delete[] buffer;
    bufsize = bufsize * 2;
    buffer = new char[bufsize];
    assert(buffer != (char *)NULL);
  }

  return Filename::from_os_specific(buffer);
}
