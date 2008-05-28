// Filename: dSearchPath.I
// Created by:  drose (01Jul00)
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


////////////////////////////////////////////////////////////////////
//     Function: DSearchPath::search_path
//       Access: Public, Static
//  Description: A quick-and-easy way to search a searchpath for a
//               file when you don't feel like building or keeping
//               around a DSearchPath object.  This simply
//               constructs a temporary DSearchPath based on the
//               indicated path string, and searches that.
////////////////////////////////////////////////////////////////////
INLINE Filename DSearchPath::
search_path(const Filename &filename, const string &path,
            const string &delimiters) {
  DSearchPath search(path, delimiters);
  return search.find_file(filename);
}
