// Filename: qpgeomCacheEntry.I
// Created by:  drose (21Mar05)
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
//     Function: qpGeomCacheEntry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomCacheEntry::
qpGeomCacheEntry() {
#ifndef NDEBUG
  _next = NULL;
  _prev = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::refresh
//       Access: Public
//  Description: Marks the cache entry recently used, so it will not
//               be evicted for a while.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomCacheEntry::
refresh() {
  nassertv(_next != (qpGeomCacheEntry *)NULL && _prev != (qpGeomCacheEntry *)NULL);

  qpGeomCacheManager *cache_mgr = qpGeomCacheManager::get_global_ptr();
  MutexHolder holder(cache_mgr->_lock);

  remove_from_list();
  insert_before(cache_mgr->_list);

  int new_size = get_result_size();
  cache_mgr->_total_size += (new_size - _result_size);
  _result_size = new_size;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::remove_from_list
//       Access: Private
//  Description: Removes a GeomCacheEntry record from the
//               doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomCacheEntry::
remove_from_list() {
  nassertv(_prev->_next == this && _next->_prev == this);
  _prev->_next = _next;
  _next->_prev = _prev;
#ifndef NDEBUG
  _next = NULL;
  _prev = NULL;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomCacheEntry::insert_before
//       Access: Private
//  Description: Adds a GeomCacheEntry record before the indicated
//               node in the doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomCacheEntry::
insert_before(qpGeomCacheEntry *node) {
  nassertv(node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == (qpGeomCacheEntry *)NULL &&
           _next == (qpGeomCacheEntry *)NULL);
  _prev = node->_prev;
  _next = node;
  _prev->_next = this;
  node->_prev = this;
}

INLINE ostream &
operator << (ostream &out, const qpGeomCacheEntry &entry) {
  entry.output(out);
  return out;
}

