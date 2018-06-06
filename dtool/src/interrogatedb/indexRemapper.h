/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indexRemapper.h
 * @author drose
 * @date 2000-08-05
 */

#ifndef INDEXREMAPPER_H
#define INDEXREMAPPER_H

#include "dtoolbase.h"

#include <map>

/**
 * This class manages a mapping of integers to integers.  It's used in this
 * package to resequence some or all of the index numbers in the database to a
 * different sequence.
 *
 * This class is just a wrapper around STL map.  The only reason it exists is
 * because Microsoft can't export STL map outside of the DLL.
 */
class EXPCL_INTERROGATEDB IndexRemapper {
public:
  IndexRemapper();
  ~IndexRemapper();

  void clear();
  void add_mapping(int from, int to);

  bool in_map(int from) const;
  int map_from(int from) const;

private:
  std::map<int, int> _map_int;
};

#endif
