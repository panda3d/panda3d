// Filename: softFilename.cxx
// Created by:  drose (10Nov00)
// 
////////////////////////////////////////////////////////////////////

#include "softFilename.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftFilename::
SoftFilename(const string &filename) :
  _filename(filename)
{
  _has_version = false;
  _major = 0;
  _minor = 0;

  // Scan for a version number and an optional extension after each
  // dot in the filename.
  size_t dot = _filename.find('.');
  while (dot != string::npos) {
    size_t m = dot + 1;
    const char *fstr = _filename.c_str();
    char *endptr;
    // Check for a numeric version number.
    int major = strtol(fstr + m , &endptr, 10);
    if (endptr != fstr + m && *endptr == '-') {
      // We got a major number, is there a minor number?
      m = (endptr - fstr) + 1;
      int minor = strtol(fstr + m, &endptr, 10);
      if (endptr != fstr + m && (*endptr == '.' || *endptr == '\0')) {
	// We got a minor number too!
	_has_version = true;
	_base = _filename.substr(0, dot + 1);
	_major = major;
	_minor = minor;
	_ext = endptr;
	return;
      }
    }

    // That wasn't a version number.  Is there more?
    dot = _filename.find('.', dot + 1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SoftFilename::
SoftFilename(const SoftFilename &copy) :
  _filename(copy._filename),
  _has_version(copy._has_version),
  _base(copy._base),
  _major(copy._major),
  _minor(copy._minor),
  _ext(copy._ext)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::Copy Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void SoftFilename::
operator = (const SoftFilename &copy) {
  _filename = copy._filename;
  _has_version = copy._has_version;
  _base = copy._base;
  _major = copy._major;
  _minor = copy._minor;
  _ext = copy._ext;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_filename
//       Access: Public
//  Description: Returns the actual filename as found in the
//               directory.
////////////////////////////////////////////////////////////////////
const string &SoftFilename::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::has_version
//       Access: Public
//  Description: Returns true if the filename had a version number,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftFilename::
has_version() const {
  return _has_version;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_1_0_filename
//       Access: Public
//  Description: Returns what the filename would be if it were version
//               1-0.
////////////////////////////////////////////////////////////////////
string SoftFilename::
get_1_0_filename() const {
  nassertr(_has_version, string());
  return _base + "1-0" + _ext;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_base
//       Access: Public
//  Description: Returns the base part of the filename.  This is
//               everything before the version number.
////////////////////////////////////////////////////////////////////
const string &SoftFilename::
get_base() const {
  nassertr(_has_version, _filename);
  return _base;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_major
//       Access: Public
//  Description: Returns the major version number.
////////////////////////////////////////////////////////////////////
int SoftFilename::
get_major() const {
  nassertr(_has_version, 0);
  return _major;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_minor
//       Access: Public
//  Description: Returns the minor version number.
////////////////////////////////////////////////////////////////////
int SoftFilename::
get_minor() const {
  nassertr(_has_version, 0);
  return _minor;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_extension
//       Access: Public
//  Description: Returns the extension part of the filename.  This is
//               everything after the version number.
////////////////////////////////////////////////////////////////////
const string &SoftFilename::
get_extension() const {
  nassertr(_has_version, _ext);
  return _ext;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::get_non_extension
//       Access: Public
//  Description: Returns the filename part, without the extension.
////////////////////////////////////////////////////////////////////
string SoftFilename::
get_non_extension() const {
  nassertr(_has_version, _filename);
  nassertr(_ext.length() < _filename.length(), _filename);
  return _filename.substr(0, _filename.length() - _ext.length());
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::is_1_0
//       Access: Public
//  Description: Returns true if this is a version 1_0 filename, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool SoftFilename::
is_1_0() const {
  nassertr(_has_version, false);
  return (_major == 1 && _minor == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::is_same_file
//       Access: Public
//  Description: Returns true if this file has the same base and
//               extension as the other, disregarding the version
//               number; false otherwise.
////////////////////////////////////////////////////////////////////
bool SoftFilename::
is_same_file(const SoftFilename &other) const {
  return _base == other._base && _ext == other._ext;
}

////////////////////////////////////////////////////////////////////
//     Function: SoftFilename::Ordering operator
//       Access: Public
//  Description: Puts filenames in order such that the files with the
//               same base and extension are sorted together; and
//               within files with the same base and exntension, files
//               are sorted in decreasing version number order so that
//               the most recent version appears first.
//
//               The ordering operator is only defined for files that
//               have a version number.
////////////////////////////////////////////////////////////////////
bool SoftFilename::
operator < (const SoftFilename &other) const {
  nassertr(_has_version, false);
  nassertr(other._has_version, false);

  if (_base != other._base) {
    return _base < other._base;
  }
  if (_ext != other._ext) {
    return _ext < other._ext;
  }
  if (_major != other._major) {
    return _major > other._major;
  }
  if (_minor != other._minor) {
    return _minor > other._minor;
  }

  return false;
}
