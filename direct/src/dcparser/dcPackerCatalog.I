// Filename: dcPackerCatalog.I
// Created by:  drose (21Jun04)
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
//     Function: DCPackerCatalog::LiveCatalog::get_unpack_p
//       Access: Public
//  Description: Returns the beginning of the indicated field within
//               the live data.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPackerCatalog::LiveCatalog::
get_unpack_p(int n) const {
  nassertr(n >= 0 && n < (int)_live_entries.size(), 0);
  return _live_entries[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::get_num_entries
//       Access: Public
//  Description: Returns the number of entries in the catalog.
////////////////////////////////////////////////////////////////////
INLINE int DCPackerCatalog::
get_num_entries() const {
  return _entries.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::get_entry
//       Access: Public
//  Description: Returns the nth entry in the catalog.
////////////////////////////////////////////////////////////////////
INLINE const DCPackerCatalog::Entry &DCPackerCatalog::
get_entry(int n) const {
  nassertr(n >= 0 && n < (int)_entries.size(), _entries[0]);
  return _entries[n];
}
