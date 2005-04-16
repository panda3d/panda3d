// Filename: qpgeomCacheManager.cxx
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

#include "qpgeomCacheManager.h"
#include "qpgeomCacheEntry.h"
#include "mutexHolder.h"

qpGeomCacheManager *qpGeomCacheManager::_global_ptr = NULL;

PStatCollector qpGeomCacheManager::_geom_cache_size_pcollector("Geom cache size");
PStatCollector qpGeomCacheManager::_geom_cache_record_pcollector("Geom cache operations:record");
PStatCollector qpGeomCacheManager::_geom_cache_erase_pcollector("Geom cache operations:erase");
PStatCollector qpGeomCacheManager::_geom_cache_evict_pcollector("Geom cache operations:evict");

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheManager::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomCacheManager::
qpGeomCacheManager() :
  _total_size(0)
{
  // We deliberately hang on to this pointer forever.
  _list = new qpGeomCacheEntry;
  _list->ref();
  _list->_next = _list;
  _list->_prev = _list;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheManager::Destructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
qpGeomCacheManager::
~qpGeomCacheManager() {
  // Shouldn't be deleting this global object.
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheManager::get_global_ptr
//       Access: Published, Static
//  Description: Returns the global cache manager pointer.
////////////////////////////////////////////////////////////////////
qpGeomCacheManager *qpGeomCacheManager::
get_global_ptr() {
  if (_global_ptr == (qpGeomCacheManager *)NULL) {
    _global_ptr = new qpGeomCacheManager;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheManager::evict_old_entries
//       Access: Private
//  Description: Trims the cache size down to get_max_size() by
//               evicting old cache entries as needed.
////////////////////////////////////////////////////////////////////
void qpGeomCacheManager::
evict_old_entries() {
  MutexHolder holder(_lock);

  int max_size = get_max_size();
  while (_total_size > max_size) {
    PT(qpGeomCacheEntry) entry = _list->_next;
    nassertv(entry != _list);
    entry->unref();

    if (gobj_cat.is_debug()) {
      gobj_cat.debug()
        << "cache total_size = " << _total_size << ", max_size = "
        << max_size << ", removing " << *entry << "\n";
    }

    entry->evict_callback();

    _total_size -= entry->_result_size;
    entry->remove_from_list();
    _geom_cache_evict_pcollector.add_level(1);
  }
  _geom_cache_size_pcollector.set_level(_total_size);
}
