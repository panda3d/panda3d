// Filename: filename.I
// Created by:  drose (18Jan99)
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

////////////////////////////////////////////////////////////////////
//     Function: Filename::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename::
Filename(const string &filename) {
  (*this) = filename;
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename::
Filename(const char *filename) {
  (*this) = filename;
  _flags = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename::
Filename(const Filename &copy)
  : _filename(copy._filename),
    _dirname_end(copy._dirname_end),
    _basename_start(copy._basename_start),
    _basename_end(copy._basename_end),
    _extension_start(copy._extension_start),
    _flags(copy._flags)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::text_filename named constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename Filename::
text_filename(const string &filename) {
  Filename result(filename);
  result.set_text();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::binary_filename named constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename Filename::
binary_filename(const string &filename) {
  Filename result(filename);
  result.set_binary();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::dso_filename named constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename Filename::
dso_filename(const string &filename) {
  Filename result(filename);
  result.set_type(T_dso);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::executable_filename named constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename Filename::
executable_filename(const string &filename) {
  Filename result(filename);
  result.set_type(T_executable);
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename::
~Filename() {
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename &Filename::
operator = (const string &filename) {
  _filename = filename;

  locate_basename();
  locate_extension();
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename &Filename::
operator = (const char *filename) {
  assert(filename != NULL);
  return (*this) = string(filename);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Copy assignment operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename &Filename::
operator = (const Filename &copy) {
  _filename = copy._filename;
  _dirname_end = copy._dirname_end;
  _basename_start = copy._basename_start;
  _basename_end = copy._basename_end;
  _extension_start = copy._extension_start;
  _flags = copy._flags;
  return *this;
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::string typecast operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE Filename::
operator const string & () const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::c_str
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE const char *Filename::
c_str() const {
  return _filename.c_str();
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::empty
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
empty() const {
  return _filename.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::length
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE size_t Filename::
length() const {
  return _filename.length();
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Indexing operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE char Filename::
operator [] (int n) const {
  assert(n >= 0 && n < (int)_filename.length());
  return _filename[n];
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::get_fullpath
//       Access: Published
//  Description: Returns the entire filename: directory, basename,
//               extension.  This is the same thing returned by the
//               string typecast operator, so this function is a
//               little redundant.
////////////////////////////////////////////////////////////////////
INLINE string Filename::
get_fullpath() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::get_dirname
//       Access: Published
//  Description: Returns the directory part of the filename.  This is
//               everything in the filename up to, but not including
//               the rightmost slash.
////////////////////////////////////////////////////////////////////
INLINE string Filename::
get_dirname() const {
  return _filename.substr(0, _dirname_end);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::get_basename
//       Access: Published
//  Description: Returns the basename part of the filename.  This is
//               everything in the filename after the rightmost slash,
//               including any extensions.
////////////////////////////////////////////////////////////////////
INLINE string Filename::
get_basename() const {
  return _filename.substr(_basename_start);
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::get_fullpath_wo_extension
//       Access: Published
//  Description: Returns the full filename--directory and basename
//               parts--except for the extension.
////////////////////////////////////////////////////////////////////
INLINE string Filename::
get_fullpath_wo_extension() const {
  return _filename.substr(0, _basename_end);
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::get_basename_wo_extension
//       Access: Published
//  Description: Returns the basename part of the filename, without
//               the file extension.
////////////////////////////////////////////////////////////////////
INLINE string Filename::
get_basename_wo_extension() const {
  if (_basename_end == string::npos) {
    return _filename.substr(_basename_start);
  } else {
    return _filename.substr(_basename_start, _basename_end - _basename_start);
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::get_extension
//       Access: Published
//  Description: Returns the file extension.  This is everything after
//               the rightmost dot, if there is one, or the empty
//               string if there is not.
////////////////////////////////////////////////////////////////////
INLINE string Filename::
get_extension() const {
  if (_extension_start == string::npos) {
    return string();
  } else {
    return _filename.substr(_extension_start);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::set_binary
//       Access: Published
//  Description: Indicates that the filename represents a binary file.
//               This is primarily relevant to the read_file() and
//               write_file() methods, so they can set the appropriate
//               flags to the OS.
////////////////////////////////////////////////////////////////////
INLINE void Filename::
set_binary() {
  _flags = (_flags & ~F_text) | F_binary;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::set_text
//       Access: Published
//  Description: Indicates that the filename represents a text file.
//               This is primarily relevant to the read_file() and
//               write_file() methods, so they can set the appropriate
//               flags to the OS.
////////////////////////////////////////////////////////////////////
INLINE void Filename::
set_text() {
  _flags = (_flags & ~F_binary) | F_text;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_binary
//       Access: Published
//  Description: Returns true if the Filename has been indicated to
//               represent a binary file via a previous call to
//               set_binary().  It is possible that neither
//               is_binary() nor is_text() will be true, if neither
//               set_binary() nor set_text() was ever called.
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
is_binary() const {
  return ((_flags & F_binary) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_text
//       Access: Published
//  Description: Returns true if the Filename has been indicated to
//               represent a text file via a previous call to
//               set_text().  It is possible that neither is_binary()
//               nor is_text() will be true, if neither set_binary()
//               nor set_text() was ever called.
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
is_text() const {
  return ((_flags & F_text) != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::set_type
//       Access: Published
//  Description: Sets the type of the file represented by the
//               filename.  This is useful for to_os_specific(),
//               resolve_filename(), test_existence(), and all such
//               real-world access functions.  It helps the Filename
//               know how to map the internal filename to the
//               OS-specific filename (for instance, maybe executables
//               should have an .exe extension).
////////////////////////////////////////////////////////////////////
INLINE void Filename::
set_type(Filename::Type type) {
  _flags = (_flags & ~F_type) | type;
  switch (type) {
  case T_dso:
  case T_executable:
    set_binary();

  case T_general:
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::get_type
//       Access: Published
//  Description: Returns the type of the file represented by the
//               filename, as previously set by set_type().
////////////////////////////////////////////////////////////////////
INLINE Filename::Type Filename::
get_type() const {
  return (Type)(_flags & (int)F_type);
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_local
//       Access: Published
//  Description: Returns true if the filename is local, e.g. does not
//               begin with a slash, or false if the filename is fully
//               specified from the root.
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
is_local() const {
  return _filename.empty() || _filename[0] != '/';
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::is_fully_qualified
//       Access: Published
//  Description: Returns true if the filename is fully qualified,
//               e.g. begins with a slash.  This is almost, but not
//               quite, the same thing as !is_local().  It's not
//               exactly the same because a special case is made for
//               filenames that begin with a single dot followed by a
//               slash--these are considered to be fully qualified
//               (they are explicitly relative to the current
//               directory, and do not refer to a filename on a search
//               path somewhere).
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
is_fully_qualified() const {
  return
    (_filename.size() > 2 && _filename[0] == '.' && _filename[1] == '/') ||
    (!_filename.empty() && _filename[0] == '/');
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Equality operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
operator == (const string &other) const {
  return (*(string *)this) == other;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Inequality operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
operator != (const string &other) const {
  return (*(string *)this) != other;
}

////////////////////////////////////////////////////////////////////
//     Function: Filename::Ordering operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool Filename::
operator < (const string &other) const {
  return (*(string *)this) < other;
}


////////////////////////////////////////////////////////////////////
//     Function: Filename::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void Filename::
output(ostream &out) const {
  out << _filename;
}
