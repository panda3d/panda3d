// Filename: indexRemapper.h
// Created by:  drose (05Aug00)
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

#ifndef INDEXREMAPPER_H
#define INDEXREMAPPER_H

#include "dtoolbase.h"

#include <map>

////////////////////////////////////////////////////////////////////
//       Class : IndexRemapper
// Description : This class manages a mapping of integers to integers.
//               It's used in this package to resequence some or all
//               of the index numbers in the database to a different
//               sequence.
//
//               This class is just a wrapper around STL map.  The
//               only reason it exists is because Microsoft can't
//               export STL map outside of the DLL.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG IndexRemapper {
public:
  IndexRemapper();
  ~IndexRemapper();

  void clear();
  void add_mapping(int from, int to);

  bool in_map(int from) const;
  int map_from(int from) const;

private:
  map<int, int> _map_int;
};

#endif
