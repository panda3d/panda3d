// Filename: find_searchpath.cxx
// Created by:  drose (09Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "find_searchpath.h"

#include <unistd.h>

string
find_searchpath(const vector<string> &directories, const string &filename) {
  vector<string>::const_iterator di;

  for (di = directories.begin(); di != directories.end(); ++di) {
    string path = (*di) + "/" + filename;
    if (access(path.c_str(), F_OK) == 0) {
      return path;
    }
  }

  return string();
}
