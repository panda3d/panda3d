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
  Entry entry;
  entry._munger = munger;
  entry._primitive = NULL;
  entry._source = NULL;
  entry._format = NULL;
  entry._result_size = 100;  // Make up a nominal number.
  
  record_entry(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::record_decompose
//       Access: Private
//  Description: Records a new entry in the cache, or marks a cache
//               hit for a previous entry in the cache.  This should
//               only be called by GeomPrimitive.
//
//               The cache manager will not hold a reference on the
//               GeomPrimitive pointer; if it destructs, it should
//               call remove_decompose() to remove itself from the cache.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
record_decompose(const qpGeomPrimitive *primitive, int result_size) {
  Entry entry;
  entry._primitive = primitive;
  entry._source = NULL;
  entry._format = NULL;
  entry._result_size = result_size;
  
  record_entry(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::remove_decompose
//       Access: Private
//  Description: Removes an entry from the cache, if it is there.
//               Quietly ignores it if it is not.  This should only be
//               called by GeomPrimitive.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
remove_decompose(const qpGeomPrimitive *primitive) {
  Entry entry;
  entry._primitive = primitive;
  entry._source = NULL;
  entry._format = NULL;

  remove_entry(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::record_data
//       Access: Private
//  Description: Records a new entry in the cache, or marks a cache
//               hit for a previous entry in the cache.  This should
//               only be called by GeomVertexData.
//
//               The cache manager will not hold a reference on the
//               GeomVertexData pointer; if it destructs, it should
//               call remove_data() to remove itself from the cache.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
record_data(const qpGeomVertexData *source, const qpGeomVertexFormat *format,
            int result_size) {
  Entry entry;
  entry._primitive = NULL;
  entry._source = source;
  entry._format = format;
  entry._result_size = result_size;
  
  record_entry(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::remove_data
//       Access: Private
//  Description: Removes an entry from the cache, if it is there.
//               Quietly ignores it if it is not.  This should only be
//               called by GeomVertexData.
////////////////////////////////////////////////////////////////////
INLINE void qpGeomVertexCacheManager::
remove_data(const qpGeomVertexData *source,
            const qpGeomVertexFormat *format) {
  Entry entry;
  entry._primitive = NULL;
  entry._source = source;
  entry._format = format;

  remove_entry(entry);
}

////////////////////////////////////////////////////////////////////
//     Function: qpGeomVertexCacheManager::Entry::operator <
//       Access: Public
//  Description: Provides a unique ordering for EntriesIndex.
////////////////////////////////////////////////////////////////////
INLINE bool qpGeomVertexCacheManager::Entry::
operator < (const qpGeomVertexCacheManager::Entry &other) const {
  if (_munger != other._munger) {
    return _munger < other._munger;
  }
  if (_primitive != other._primitive) {
    return _primitive < other._primitive;
  }
  if (_source != other._source) {
    return _source < other._source;
  }
  return _format < other._format;
}
