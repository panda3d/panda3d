// Filename: dSearchPath.I
// Created by:  drose (01Jul00)
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
