// Filename: nameUniquifier.h
// Created by:  drose (16Feb00)
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

#ifndef NAMEUNIQUIFIER_H
#define NAMEUNIQUIFIER_H

#include "pandabase.h"

#include <string>
#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : NameUniquifier
// Description : A handy class for converting a list of arbitrary
//               names (strings) so that each name is guaranteed to be
//               unique in the list.  Useful for writing egg files
//               with unique vertex pool names, or for file converters
//               to file formats that require unique node names, etc.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA NameUniquifier {
public:
  NameUniquifier(const string &separator = string(),
                 const string &empty = string());
  ~NameUniquifier();

  INLINE string add_name(const string &name);
  INLINE string add_name(const string &name, const string &prefix);

private:
  string add_name_body(const string &name, const string &prefix);

  typedef pset<string, string_hash> Names;
  Names _names;
  string _separator;
  string _empty;
  int _counter;
};

#include "nameUniquifier.I"

#endif
