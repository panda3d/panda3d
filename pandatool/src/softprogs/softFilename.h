// Filename: softFilename.h
// Created by:  drose (10Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef SOFTFILENAME_H
#define SOFTFILENAME_H

#include "pandatoolbase.h"

////////////////////////////////////////////////////////////////////
//       Class : SoftFilename
// Description : This encapsulates a SoftImage versioned filename, of
//               the form base.v-v.ext: it consists of a directory
//               name, a base, a major and minor version number, and
//               an optional extension.
//
//               It also keeps track of whether the named file has
//               been added to CVS, and how many scene files it is
//               referenced by,
////////////////////////////////////////////////////////////////////
class SoftFilename {
public:
  SoftFilename(const string &dirname, const string &filename);
  SoftFilename(const SoftFilename &copy);
  void operator = (const SoftFilename &copy);

  const string &get_dirname() const;
  const string &get_filename() const;
  bool has_version() const;

  string get_1_0_filename() const;

  const string &get_base() const;
  int get_major() const;
  int get_minor() const;
  const string &get_extension() const;
  string get_non_extension() const;

  bool is_1_0() const;
  void make_1_0();

  bool is_same_file(const SoftFilename &other) const;
  bool operator < (const SoftFilename &other) const;

  void set_in_cvs(bool in_cvs);
  bool get_in_cvs() const;

  void set_wants_cvs(bool wants_cvs);
  bool get_wants_cvs() const;

  void increment_use_count();
  int get_use_count() const;

private:
  string _dirname;
  string _filename;
  bool _has_version;
  string _base;
  int _major;
  int _minor;
  string _ext;
  bool _in_cvs;
  bool _wants_cvs;
  int _use_count;
};

#endif
