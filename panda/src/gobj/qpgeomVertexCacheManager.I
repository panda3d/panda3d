// Filename: qpgeomVertexCacheManager.I
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


////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::set_max_size
//       Access: Published
//  Description: Specifies the amount of memory, in bytes, that should
//               be set aside for storing pre-processed data for
//               rendering vertices.  This is not a limit on the
//               actual vertex data, which is what it is; it is also
//               not a limit on the amount of memory used by the video
//               driver or the system graphics interface, which Panda
//               has no control over.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
set_max_size(int max_size) const {
  // We directly change the config variable.
  vertex_convert_cache = max_size;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::get_max_size
//       Access: Published
//  Description: Returns the amount of memory, in bytes, that should
//               be set aside for storing pre-processed data for
//               rendering vertices.  See set_max_size().
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexCacheManager::
get_max_size() const {
  return vertex_convert_cache;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::get_total_size
//       Access: Published
//  Description: Returns the amount of memory, in bytes, currently
//               consumed by the cache of pre-processed vertex data.
////////////////////////////////////////////////////////////////////
INLINE int qpGeomVertexCacheManager::
get_total_size() const {
  return _total_size;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::record_munger
//       Access: Private
//  Description: Records a new GeomMunger in the cache, or marks a
//               cache hit for a previously-recorded munger.  This
//               should only be called by GeomMunger.
//
//               The cache manager will hold a reference on the
//               GeomMunger pointer until it expires from the cache.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
record_munger(const qpGeomMunger *munger) {
  // Make up a nominal result_size for a munger.
  record_entry(Entry(munger, 100));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::record_primitive
//       Access: Private
//  Description: Records a new entry in the cache, or marks a cache
//               hit for a previous entry in the cache.  This should
//               only be called by GeomPrimitive.
//
//               The cache manager will not hold a reference on the
//               GeomPrimitive pointer; if it destructs, it should
//               call remove_primitive() to remove itself from the cache.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
record_primitive(const qpGeomPrimitive *primitive, int result_size) {
  record_entry(Entry(primitive, result_size));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::remove_primitive
//       Access: Private
//  Description: Removes an entry from the cache, if it is there.
//               Quietly ignores it if it is not.  This should only be
//               called by GeomPrimitive.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
remove_primitive(const qpGeomPrimitive *primitive) {
  remove_entry(Entry(primitive, 0));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::record_geom
//       Access: Private
//  Description: Records a new entry in the cache, or marks a cache
//               hit for a previous entry in the cache.  This should
//               only be called by Geom.
//
//               The cache manager will not hold a reference on the
//               Geom pointer; if it destructs, it should call
//               remove_geom() to remove itself from the cache.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
record_geom(const qpGeom *source, const qpGeomMunger *modifier,
            int result_size) {
  record_entry(Entry(source, modifier, result_size));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::remove_geom
//       Access: Private
//  Description: Removes an entry from the cache, if it is there.
//               Quietly ignores it if it is not.  This should only be
//               called by GeomVertexData.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
remove_geom(const qpGeom *source, const qpGeomMunger *modifier) {
  remove_entry(Entry(source, modifier, 0));
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::dequeue_entry
//       Access: Private
//  Description: Removes an Entry record from the doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
dequeue_entry(qpGeomVertexCacheManager::Entry *entry) {
  nassertv(entry->_prev->_next == entry &&
           entry->_next->_prev == entry);
  entry->_prev->_next = entry->_next;
  entry->_next->_prev = entry->_prev;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::enqueue_entry
//       Access: Private
//  Description: Adds an Entry record to the tail of the doubly-linked
//               list.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
enqueue_entry(qpGeomVertexCacheManager::Entry *entry) {
  nassertv(_list->_prev->_next == _list &&
           _list->_next->_prev == _list);
  entry->_prev = _list->_prev;
  entry->_next = _list;
  entry->_prev->_next = entry;
  _list->_prev = entry;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexCacheManager::Entry::
Entry() :
  _cache_type(CT_none),
  _result_size(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexCacheManager::Entry::
Entry(const qpGeomMunger *munger, int result_size) :
  _cache_type(CT_munger),
  _result_size(result_size)
{
  _u._munger = munger;
  _u._munger->ref();
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexCacheManager::Entry::
Entry(const qpGeomPrimitive *primitive, int result_size) :
  _cache_type(CT_primitive),
  _result_size(result_size)
{
  _u._primitive = primitive;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexCacheManager::Entry::
Entry(const qpGeom *source, const qpGeomMunger *modifier,
      int result_size) :
  _cache_type(CT_geom),
  _result_size(result_size)
{
  _u._geom._source = source;
  _u._geom._modifier = modifier;
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexCacheManager::Entry::
Entry(const qpGeomVertexCacheManager::Entry &copy) :
  _cache_type(copy._cache_type),
  _result_size(copy._result_size)
{
  _u = copy._u;
  if (_cache_type == CT_munger) {
    _u._munger->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::Entry::
operator = (const qpGeomVertexCacheManager::Entry &copy) {
  if (_cache_type == CT_munger) {
    unref_delete(_u._munger);
  }
  _cache_type = copy._cache_type;
  _result_size = copy._result_size;
  _u = copy._u;
  if (_cache_type == CT_munger) {
    _u._munger->ref();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE qpGeomVertexCacheManager::Entry::
~Entry() {
  if (_cache_type == CT_munger) {
    unref_delete(_u._munger);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::operator <
//       Access: Public
//  Description: Provides a unique ordering for EntriesIndex.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexCacheManager::Entry::
operator < (const qpGeomVertexCacheManager::Entry &other) const {
  if (_cache_type != other._cache_type) {
    return (int)_cache_type < (int)other._cache_type;
  }

  switch (_cache_type) {
  case CT_none:
    // We shouldn't be adding the end-of-list token to the index.
    nassertr(false, false);
    return false;

  case CT_munger:
    return _u._munger < other._u._munger;

  case CT_primitive:
    return _u._primitive < other._u._primitive;

  case CT_geom:
    if (_u._geom._source != other._u._geom._source) {
      return _u._geom._source < other._u._geom._source;
    }
    return _u._geom._modifier < other._u._geom._modifier;
  }

  return false;
}

INLINE ostream &
operator << (ostream &out, const qpGeomVertexCacheManager::Entry &entry) {
  entry.output(out);
  return out;
}

