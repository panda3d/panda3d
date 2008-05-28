// Filename: dcPackerCatalog.I
// Created by:  drose (21Jun04)
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
//     Function: DCPackerCatalog::LiveCatalog::get_begin
//       Access: Public
//  Description: Returns the beginning of the indicated field within
//               the live data.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPackerCatalog::LiveCatalog::
get_begin(int n) const {
  nassertr(n >= 0 && n < (int)_live_entries.size(), 0);
  return _live_entries[n]._begin;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::LiveCatalog::get_end
//       Access: Public
//  Description: Returns the end of the indicated field (the byte
//               position of the first following field) within the
//               live data.
////////////////////////////////////////////////////////////////////
INLINE size_t DCPackerCatalog::LiveCatalog::
get_end(int n) const {
  nassertr(n >= 0 && n < (int)_live_entries.size(), 0);
  return _live_entries[n]._end;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::LiveCatalog::get_num_entries
//       Access: Public
//  Description: Returns the number of entries in the catalog.
////////////////////////////////////////////////////////////////////
INLINE int DCPackerCatalog::LiveCatalog::
get_num_entries() const {
  return _catalog->get_num_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::LiveCatalog::get_entry
//       Access: Public
//  Description: Returns the nth entry in the catalog.
////////////////////////////////////////////////////////////////////
INLINE const DCPackerCatalog::Entry &DCPackerCatalog::LiveCatalog::
get_entry(int n) const {
  return _catalog->get_entry(n);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::LiveCatalog::find_entry_by_name
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated name, or -1 if no entry has the indicated
//               name.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int DCPackerCatalog::LiveCatalog::
find_entry_by_name(const string &name) const {
  return _catalog->find_entry_by_name(name);
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::LiveCatalog::find_entry_by_field
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated field, or -1 if no entry has the indicated
//               field.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int DCPackerCatalog::LiveCatalog::
find_entry_by_field(const DCPackerInterface *field) const {
  return _catalog->find_entry_by_field(field);
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
