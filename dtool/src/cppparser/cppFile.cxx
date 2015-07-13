// Filename: cppFile.cxx
// Created by:  drose (11Nov99)
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


#include "cppFile.h"

#include <ctype.h>

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFile::
CPPFile(const Filename &filename, const Filename &filename_as_referenced,
        Source source) :
  _filename(filename), _filename_as_referenced(filename_as_referenced),
  _source(source),
  _pragma_once(false)
{
  _filename.set_text();
  _filename_as_referenced.set_text();
}


////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFile::
CPPFile(const CPPFile &copy) :
  _filename(copy._filename),
  _filename_as_referenced(copy._filename_as_referenced),
  _source(copy._source),
  _pragma_once(copy._pragma_once)
{
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void CPPFile::
operator = (const CPPFile &copy) {
  _filename = copy._filename;
  _filename_as_referenced = copy._filename_as_referenced;
  _source = copy._source;
  _pragma_once = copy._pragma_once;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
CPPFile::
~CPPFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::is_c_or_i_file
//       Access: Public
//  Description: Returns true if the file appears to be a C or C++
//               source code file based on its extension.  That is,
//               returns true if the filename ends in .c, .C, .cc,
//               .cpp, or any of a series of likely extensions.
////////////////////////////////////////////////////////////////////
bool CPPFile::
is_c_or_i_file() const {
  return is_c_or_i_file(_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::is_c_or_i_file
//       Access: Public, Static
//  Description: Returns true if the file appears to be a C or C++
//               source code file based on its extension.  That is,
//               returns true if the filename ends in .c, .C, .cc,
//               .cpp, or any of a series of likely extensions.
////////////////////////////////////////////////////////////////////
bool CPPFile::
is_c_or_i_file(const Filename &filename) {
  string extension = filename.get_extension();
  // downcase the extension.
  for (string::iterator ei = extension.begin();
       ei != extension.end();
       ++ei) {
    (*ei) = tolower(*ei);
  }

  return (extension == "c" || extension == "cc" ||
          extension == "cpp" || extension == "c++" || extension == "cxx" ||
          extension == "i" || extension == "t");
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::is_c_file
//       Access: Public
//  Description: Returns true if the file appears to be a C or C++
//               source code file based on its extension.  That is,
//               returns true if the filename ends in .c, .C, .cc,
//               .cpp, or any of a series of likely extensions.
////////////////////////////////////////////////////////////////////
bool CPPFile::
is_c_file() const {
  return is_c_file(_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::is_c_file
//       Access: Public, Static
//  Description: Returns true if the file appears to be a C or C++
//               source code file based on its extension.  That is,
//               returns true if the filename ends in .c, .C, .cc,
//               .cpp, or any of a series of likely extensions.
////////////////////////////////////////////////////////////////////
bool CPPFile::
is_c_file(const Filename &filename) {
  string extension = filename.get_extension();
  // downcase the extension.
  for (string::iterator ei = extension.begin();
       ei != extension.end();
       ++ei) {
    (*ei) = tolower(*ei);
  }

  return (extension == "c" || extension == "cc" ||
          extension == "cpp" || extension == "c++" || extension == "cxx");
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::replace_nearer
//       Access: Public
//  Description: If the other file is "nearer" than this file (in the
//               sense that a file in the local directory is nearer
//               than a file in the system directory, etc.), replaces
//               this file's information with that of the other.
//               Otherwise, does nothing.
////////////////////////////////////////////////////////////////////
void CPPFile::
replace_nearer(const CPPFile &other) {
  if ((int)_source > (int)other._source) {
    (*this) = other;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Ordering Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPFile::
operator < (const CPPFile &other) const {
  return _filename < other._filename;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Equality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPFile::
operator == (const CPPFile &other) const {
  return _filename == other._filename;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::Inequality Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPFile::
operator != (const CPPFile &other) const {
  return _filename != other._filename;
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::c_str
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
const char *CPPFile::
c_str() const {
  return _filename.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: CPPFile::empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool CPPFile::
empty() const {
  return _filename.empty();
}
