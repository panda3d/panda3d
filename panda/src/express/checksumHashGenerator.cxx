// Filename: checksumHashGenerator.cxx
// Created by:  drose (14May01)
// 
////////////////////////////////////////////////////////////////////


#include "checksumHashGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: ChecksumHashGenerator::add_string
//       Access: Public
//  Description: Adds a string to the hash, by breaking it down into a
//               sequence of integers.
////////////////////////////////////////////////////////////////////
void ChecksumHashGenerator::
add_string(const string &str) {
  add_int(str.length());
  string::const_iterator si;
  for (si = str.begin(); si != str.end(); ++si) {
    add_int(*si);
  }
}
