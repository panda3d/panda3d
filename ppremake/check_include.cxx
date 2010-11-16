// Filename: check_include.cxx
// Created by:  drose (16Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "check_include.h"

////////////////////////////////////////////////////////////////////
//     Function: check_include
//  Description: Checks to see if the given line is a C/C++ #include
//               directive.  If it is, returns the named filename;
//               otherwise, returns the empty string.
////////////////////////////////////////////////////////////////////
string
check_include(const string &line) {
  // Skip initial whitespace on the line.
  size_t p = 0;
  while (p < line.length() && isspace(line[p])) {
    p++;
  }

  if (p >= line.length() || line[p] != '#') {
    // No hash mark.
    return string();
  }
   
  // We have a hash mark!  Skip more whitespace.
  p++;
  while (p < line.length() && isspace(line[p])) {
    p++;
  }

  if (p >= line.length() || !(line.substr(p, 7) == "include")) {
    // Some other directive, not #include.
    return string();
  }

  // It's an #include directive!  Skip more whitespace.
  p += 7;
  while (p < line.length() && isspace(line[p])) {
    p++;
  }

  // note: ppremake cant expand cpp #define vars used as include targets yet

  if (p >= line.length() || (line[p] != '"' && line[p] != '<')) {
    // if it starts with a capital, assume its a #define var used as
    // include tgt, and don't print a warning
    if(!((line[p]>='A')&&(line[p]<='Z'))) {
      cerr << "Ignoring invalid #include directive: " << line << "\n";
    }
    return string();
  }

  char close = (line[p] == '"') ? '"' : '>';

  p++;
  // Now get the filename.
  size_t q = p;
  while (q < line.length() && line[q] != close) {
    q++;
  }

  if (q >= line.length()) {
    cerr << "Ignoring invalid #include directive: " << line << "\n";
    return string();
  }

  return line.substr(p, q - p);
}
