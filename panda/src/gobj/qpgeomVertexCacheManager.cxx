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
  _list = new Entry;
  _list->_next = _list;
  _list->_prev = _list;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomVertexCacheManager::
~qpGeomVertexCacheManager() {
  // Shouldn't be deleting this global object.
  nassertv(false);
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
record_entry(const qpGeomVertexCacheManager::Entry &const_entry) {
  MutexHolder holder(_lock);

  EntriesIndex::iterator ii = _entries_index.find((Entry *)&const_entry);
  if (ii != _entries_index.end()) {
    // We already had this entry in the cache.  Refresh it.
    if (gobj_cat.is_spam()) {
      gobj_cat.spam()
        << "refreshing cache entry: " << const_entry << "\n";
    }

    // Move the previous cache entry to the tail of the list.
    Entry *entry = (*ii);
    dequeue_entry(entry);
    enqueue_entry(entry);

    _total_size += (const_entry._result_size - entry->_result_size);
    entry->_result_size = const_entry._result_size;

  } else {
    // There was no such entry already in the cache.  Add it.
    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "recording cache entry: " << const_entry << ", total_size = "
        << _total_size + const_entry._result_size << "\n";
    }
    
    Entry *entry = new Entry(const_entry);
    enqueue_entry(entry);
    _total_size += entry->_result_size;

    // Also record an index entry.
    bool inserted = _entries_index.insert(entry).second;
    nassertv(inserted);
  }

  // Now remove any old entries if our cache is over the limit.  This may
  // also remove the entry we just added, especially if our cache size
  // is set to 0.
  int max_size = get_max_size();
  while (_total_size > max_size) {
    Entry *entry = _list->_next;
    nassertv(entry != _list);

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "cache total_size = " << _total_size << ", max_size = "
        << max_size << ", removing " << *entry << "\n";
    }

    ii = _entries_index.find(entry);
    nassertv(ii != _entries_index.end());

    switch (entry->_cache_type) {
    case CT_primitive:
      entry->_u._primitive->remove_cache_entry();
      break;

    case CT_geom:
      entry->_u._geom._source->remove_cache_entry
        (entry->_u._geom._modifier);
      break;

    default:
      break;
    }
    _total_size -= entry->_result_size;
    _entries_index.erase(ii);

    dequeue_entry(entry);
    delete entry;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::remove_entry
//       Access: Private
//  Description: Removes an entry from the cache, if it is there.
//               Quietly ignores it if it is not.
////////////////////////////////////////////////////////////////////
void qpGeomVertexCacheManager::
remove_entry(const qpGeomVertexCacheManager::Entry &const_entry) {
  if (gobj_cat.is_debug()) {
    gobj_cat.debug()
      << "remove_entry(" << const_entry << ")\n";
  }
  MutexHolder holder(_lock);

  EntriesIndex::iterator ii = _entries_index.find((Entry *)&const_entry);
  if (ii != _entries_index.end()) {
    Entry *entry = (*ii);
    _total_size -= entry->_result_size;
    _entries_index.erase(ii);
    dequeue_entry(entry);
    delete entry;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void qpGeomVertexCacheManager::Entry::
output(ostream &out) const {
  out << "[ ";
  switch (_cache_type) {
  case CT_none:
    out << "end-of-list token";
    break;

  case CT_munger:
    out << "munger " << (void *)_u._munger << ":" 
        << _u._munger->get_ref_count();
    break;

  case CT_primitive:
    out << "primitive " << (void *)_u._primitive;
    break;

  case CT_geom:
    out << "geom " << (void *)_u._geom._source << ", " 
        << (void *)_u._geom._modifier;
    break;
  }

  out << ", result_size = " << _result_size << " ]";  
}
