// Filename: qpgeomVertexCacheManager.cxx
// Created by:  drose (11Mar05)
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

#include "qpgeomVertexCacheManager.h"
#include "mutexHolder.h"

qpGeomVertexCacheManager *qpGeomVertexCacheManager::_global_ptr = NULL;

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexCacheManager::
qpGeomVertexCacheManager() :
  _total_size(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexCacheManager::
~qpGeomVertexCacheManager() {
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::get_global_ptr
//       Access: Published, Static
//  Description: Returns the global cache manager pointer.
////////////////////////////////////////////////////////////////////
qpGeomVertexCacheManager *qpGeomVertexCacheManager::
get_global_ptr() {
  if (_global_ptr == (qpGeomVertexCacheManager *)NULL) {
    _global_ptr = new qpGeomVertexCacheManager;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::record_data
//       Access: Private
//  Description: Records a new generic entry in the cache, or marks a
//               cache hit for a previous entry in the cache.
////////////////////////////////////////////////////////////////////
void qpGeomVertexCacheManager::
record_entry(const qpGeomVertexCacheManager::Entry &entry) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "record_entry(" << entry._munger << ", " 
      << entry._primitive << ", " << entry._source << ", " 
      << entry._format << ", " << entry._result_size << ")\n";
  }

  MutexHolder holder(_lock);
  EntriesIndex::iterator ii = _entries_index.find(&entry);
  if (ii != _entries_index.end()) {
    // Remove the previous cache entry.
    Entries::iterator ei = (*ii).second;
    _total_size -= (*ei)._result_size;
    _entries_index.erase(ii);
    _entries.erase(ei);
  }

  // Add the new entry at the end of the list, so it will be the last
  // to be removed.
  Entries::iterator ei = 
    _entries.insert(_entries.end(), entry);
  Entry *entry_pointer = &(*ei);

  // Also record an index entry.
  bool inserted = _entries_index.insert
    (EntriesIndex::value_type(entry_pointer, ei)).second;
  nassertv(inserted);

  _total_size += entry_pointer->_result_size;

  // Now remove any old entries if our cache is over the limit.  This may
  // also remove the entry we just added, especially if our cache size
  // is set to 0.
  int max_size = get_max_size();
  while (_total_size > max_size) {
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "total_size = " << _total_size << ", max_size = "
        << max_size << ", flushing\n";
    }
    nassertv(!_entries.empty());
    ei = _entries.begin();
    entry_pointer = &(*ei);

    ii = _entries_index.find(entry_pointer);
    nassertv(ii != _entries_index.end());

    if (entry_pointer->_source != (qpGeomVertexData *)NULL) {
      entry_pointer->_source->remove_cache_entry(entry_pointer->_format);
    }
    _total_size -= entry_pointer->_result_size;
    _entries_index.erase(ii);
    _entries.erase(ei);
  }
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "total_size = " << _total_size << ", max_size = "
      << max_size << ", done\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::remove_entry
//       Access: Private
//  Description: Removes an entry from the cache, if it is there.
//               Quietly ignores it if it is not.
////////////////////////////////////////////////////////////////////
void qpGeomVertexCacheManager::
remove_entry(const qpGeomVertexCacheManager::Entry &entry) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "remove_entry(" << entry._munger << ", "
      << entry._source << ", " << entry._format << ")\n";
  }
  MutexHolder holder(_lock);

  EntriesIndex::iterator ii = _entries_index.find(&entry);
  if (ii != _entries_index.end()) {
    Entries::iterator ei = (*ii).second;
    _total_size -= (*ei)._result_size;
    _entries_index.erase(ii);
    _entries.erase(ei);
  }
}
