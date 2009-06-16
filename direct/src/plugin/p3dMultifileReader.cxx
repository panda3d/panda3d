// Filename: p3dMultifileReader.cxx
// Created by:  drose (15Jun09)
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

#include "p3dMultifileReader.h"

// This sequence of bytes begins each Multifile to identify it as a
// Multifile.
const char P3DMultifileReader::_header[] = "pmf\0\n\r";
const size_t P3DMultifileReader::_header_size = 6;

const int P3DMultifileReader::_current_major_ver = 1;
const int P3DMultifileReader::_current_minor_ver = 1;

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DMultifileReader::
P3DMultifileReader() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::extract
//       Access: Public
//  Description: Reads the named multifile, and extracts all files
//               within it to the indicated directory.  Returns true
//               on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
extract(const string &pathname, const string &to_dir) {
  _subfiles.clear();

  _in.open(pathname.c_str(), ios::in | ios::binary);
  if (!_in) {
    cerr << "Couldn't open " << pathname << "\n";
    return false;
  }

  for (size_t i = 0; i < _header_size; ++i) {
    int ch = _in.get();
    if (ch != _header[i]) {
      cerr << "Failed header check: " << pathname << "\n";
      return false;
    }
  }

  unsigned int major = read_uint16();
  unsigned int minor = read_uint16();
  if (major != _current_major_ver || minor != _current_minor_ver) {
    cerr << "Incompatible multifile version: " << pathname << "\n";
    return false;
  }

  unsigned int scale = read_uint32();
  if (scale != 1) {
    cerr << "Unsupported scale factor in " << pathname << "\n";
    return false;
  }

  // We don't care about the timestamp.
  read_uint32();

  if (!read_index()) {
    cerr << "Error reading multifile index\n";
    return false;
  }

  // Now walk through all of the files.
  Subfiles::iterator si;
  for (si = _subfiles.begin(); si != _subfiles.end(); ++si) {
    const Subfile &s = (*si);
    cerr << s._filename << "\n";

    string output_pathname = to_dir + "/" + s._filename;
    ofstream out(output_pathname.c_str(), ios::out | ios::trunc | ios::binary);
    if (!out) {
      cerr << "Unable to create " << output_pathname << "\n";
      return false;
    }

    _in.seekg(s._start);

    static const size_t buffer_size = 1024;
    char buffer[buffer_size];

    size_t remaining_data = s._length;
    _in.read(buffer, min(buffer_size, remaining_data));
    size_t count = _in.gcount();
    while (count != 0) {
      remaining_data -= count;
      out.write(buffer, count);
      _in.read(buffer, min(buffer_size, remaining_data));
      count = _in.gcount();
    }

    if (remaining_data != 0) {
      cerr << "Unable to extract " << s._filename << "\n";
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DMultifileReader::read_index
//       Access: Public
//  Description: Assuming the file stream is positioned at the first
//               record, reads all of the records into the _subfiles
//               list.  Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DMultifileReader::
read_index() {
  unsigned int next_entry = read_uint32();
  if (!_in) {
    return false;
  }
  while (next_entry != 0) {
    Subfile s;
    s._start = read_uint32();
    s._length = read_uint32();
    unsigned int flags = read_uint16();
    if (flags != 0) {
      cerr << "Unsupported per-subfile options in multifile\n";
      return false;
    }
    read_uint32();
    size_t name_length = read_uint16();
    char *buffer = new char[name_length];
    _in.read(buffer, name_length);

    // The filenames are xored with 0xff just for fun.
    for (size_t ni = 0; ni < name_length; ++ni) {
      buffer[ni] ^= 0xff;
    }

    s._filename = string(buffer, name_length);
    delete[] buffer;

    _subfiles.push_back(s);

    _in.seekg(next_entry);
    next_entry = read_uint32();
    if (!_in) {
      return false;
    }
  }

  return true;
}
