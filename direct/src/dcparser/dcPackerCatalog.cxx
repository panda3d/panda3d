// Filename: dcPackerCatalog.cxx
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

#include "dcPackerCatalog.h"
#include "dcPackerInterface.h"
#include "dcPacker.h"

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::Constructor
//       Access: Private
//  Description: The catalog is created only by
//               DCPackerInterface::get_catalog().
////////////////////////////////////////////////////////////////////
DCPackerCatalog::
DCPackerCatalog(const DCPackerInterface *root) : _root(root) {
  _live_catalog = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::Destructor
//       Access: Private
//  Description: The catalog is destroyed only by
//               ~DCPackerInterface().
////////////////////////////////////////////////////////////////////
DCPackerCatalog::
~DCPackerCatalog() {
  if (_live_catalog != (LiveCatalog *)NULL) {
    delete _live_catalog;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::find_entry_by_name
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated name, or -1 if no entry has the indicated
//               name.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int DCPackerCatalog::
find_entry_by_name(const string &name) const {
  EntriesByName::const_iterator ni;
  ni = _entries_by_name.find(name);
  if (ni != _entries_by_name.end()) {
    return (*ni).second;
  }
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::find_entry_by_field
//       Access: Public
//  Description: Returns the index number of the entry with the
//               indicated field, or -1 if no entry has the indicated
//               field.  The return value is suitable for passing to
//               get_entry().
////////////////////////////////////////////////////////////////////
int DCPackerCatalog::
find_entry_by_field(const DCPackerInterface *field) const {
  EntriesByField::const_iterator ni;
  ni = _entries_by_field.find(field);
  if (ni != _entries_by_field.end()) {
    return (*ni).second;
  }
  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::get_live_catalog
//       Access: Public
//  Description: Returns a LiveCatalog object indicating the positions
//               within the indicated data record of each field within
//               the catalog.  If the catalog's fields are all
//               fixed-width, this may return a statically-allocated
//               LiveCatalog object that is the same for all data
//               records; otherwise, it will allocate a new
//               LiveCatalog object that must be freed with a later
//               call to release_live_catalog().
////////////////////////////////////////////////////////////////////
const DCPackerCatalog::LiveCatalog *DCPackerCatalog::
get_live_catalog(const char *data, size_t length) const {
  if (_live_catalog != (LiveCatalog *)NULL) {
    // Return the previously-allocated static catalog.
    return _live_catalog;
  }

  LiveCatalog *live_catalog = new LiveCatalog;
  live_catalog->_live_entries.reserve(_entries.size());
  LiveCatalogEntry zero_entry;
  zero_entry._begin = 0;
  zero_entry._end = 0;
  for (size_t i = 0; i < _entries.size(); i++) {
    live_catalog->_live_entries.push_back(zero_entry);
  }
  
  DCPacker packer;
  packer.begin_unpack(data, length, _root);
  r_fill_live_catalog(live_catalog, packer);
  bool okflag = packer.end_unpack();

  nassertr(okflag, live_catalog);

  if (_root->has_fixed_byte_size()) {
    // If our root field has a fixed byte size, then the live catalog
    // will always be the same every time, so we might as well keep
    // this one around as an optimization.
    ((DCPackerCatalog *)this)->_live_catalog = live_catalog;
  }

  return live_catalog;
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::release_live_catalog
//       Access: Public
//  Description: Releases the LiveCatalog object that was returned by
//               an earlier call to get_live_catalog().  If this
//               represents a newly-allocated live catalog, it will
//               free it; otherwise, it will do nothing.
//
//               It is therefore always correct (and necessary) to
//               match a call to get_live_catalog() with a later call
//               to release_live_catalog().
////////////////////////////////////////////////////////////////////
void DCPackerCatalog::
release_live_catalog(const DCPackerCatalog::LiveCatalog *live_catalog) const {
  if (live_catalog != _live_catalog) {
    delete (LiveCatalog *)live_catalog;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::add_entry
//       Access: Private
//  Description: Called only by DCPackerInterface::r_fill_catalog(),
//               this adds a new entry to the catalog.
////////////////////////////////////////////////////////////////////
void DCPackerCatalog::
add_entry(const string &name, const DCPackerInterface *field,
          const DCPackerInterface *parent, int field_index) {
  Entry entry;
  entry._name = name;
  entry._field = field;
  entry._parent = parent;
  entry._field_index = field_index;

  int entry_index = (int)_entries.size();
  _entries.push_back(entry);
  _entries_by_name.insert(EntriesByName::value_type(name, entry_index));
  _entries_by_field.insert(EntriesByField::value_type(field, entry_index));
}

////////////////////////////////////////////////////////////////////
//     Function: DCPackerCatalog::r_fill_live_catalog
//       Access: Private
//  Description: Recursively walks through all of the fields on the
//               catalog and fills the live catalog with the
//               appropriate offsets.
////////////////////////////////////////////////////////////////////
void DCPackerCatalog::
r_fill_live_catalog(LiveCatalog *live_catalog, DCPacker &packer) const {
  const DCPackerInterface *current_field = packer.get_current_field();

  int field_index = find_entry_by_field(current_field);
  if (field_index >= 0) {
    live_catalog->_live_entries[field_index]._begin = packer.get_num_unpacked_bytes();
  }

  if (packer.has_nested_fields() && packer.get_pack_type() != PT_string) {
    packer.push();
    while (packer.more_nested_fields()) {
      r_fill_live_catalog(live_catalog, packer);
    }
    packer.pop();

  } else {
    packer.unpack_skip();
  }

  if (field_index >= 0) {
    live_catalog->_live_entries[field_index]._end = packer.get_num_unpacked_bytes();
  }
}
