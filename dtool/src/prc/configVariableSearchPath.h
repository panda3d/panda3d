// Filename: configVariableSearchPath.h
// Created by:  drose (21Oct04)
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

#ifndef CONFIGVARIABLESEARCHPATH_H
#define CONFIGVARIABLESEARCHPATH_H

#include "dtoolbase.h"
#include "configVariableBase.h"
#include "dSearchPath.h"

////////////////////////////////////////////////////////////////////
//       Class : ConfigVariableSearchPath
// Description : This is similar to a ConfigVariableList, but it
//               returns its list as a DSearchPath, as a list of
//               directories.
//
//               You may locally append directories to the end of the
//               search path with the methods here, or prepend them to
//               the beginning.  Use these methods to make adjustments
//               to the path; do not attempt to directly modify the
//               const DSearchPath object returned by get_value().
//
//               Unlike other ConfigVariable types, local changes
//               (made by calling append_directory() and
//               prepend_directory()) are specific to this particular
//               instance of the ConfigVariableSearchPath.  A separate
//               instance of the same variable, created by using the
//               same name to the constructor, will not reflect the
//               local changes.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG ConfigVariableSearchPath : public ConfigVariableBase {
PUBLISHED:
  ConfigVariableSearchPath(const string &name, int flags = 0,
                           const string &description = string());
  INLINE ~ConfigVariableSearchPath();

  INLINE operator const DSearchPath & () const;
  const DSearchPath &get_value() const;

  INLINE bool clear_local_value();

  INLINE void clear();
  INLINE void append_directory(const Filename &directory);
  INLINE void prepend_directory(const Filename &directory);
  INLINE void append_path(const string &path,
                          const string &separator = string());
  INLINE void append_path(const DSearchPath &path);
  INLINE void prepend_path(const DSearchPath &path);

  INLINE bool is_empty() const;
  INLINE int get_num_directories() const;
  INLINE const Filename &get_directory(int n) const;

  INLINE Filename find_file(const Filename &filename) const;
  INLINE int find_all_files(const Filename &filename, 
                            DSearchPath::Results &results) const;

  INLINE void output(ostream &out) const;
  INLINE void write(ostream &out) const;

private:
  void reload_search_path();

  int _value_seq;
  bool _value_stale;

  DSearchPath _value;
  DSearchPath _prefix, _postfix;
};

INLINE ostream &operator << (ostream &out, const ConfigVariableSearchPath &variable);

#include "configVariableSearchPath.I"

#endif
