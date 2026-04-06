/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomCacheEntry.I
 * @author drose
 * @date 2005-03-21
 */

/**
 *
 */
INLINE GeomCacheEntry::
GeomCacheEntry() {
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}

/**
 * Removes a GeomCacheEntry record from the doubly-linked list.
 */
INLINE void GeomCacheEntry::
remove_from_list() {
  nassertv(_prev->_next == this && _next->_prev == this);
  _prev->_next = _next;
  _next->_prev = _prev;
#ifndef NDEBUG
  _next = nullptr;
  _prev = nullptr;
#endif
}

/**
 * Adds a GeomCacheEntry record before the indicated node in the doubly-linked
 * list.
 */
INLINE void GeomCacheEntry::
insert_before(GeomCacheEntry *node) {
  nassertv(node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == nullptr &&
           _next == nullptr);
  _prev = node->_prev;
  _next = node;
  _prev->_next = this;
  node->_prev = this;
}

INLINE std::ostream &
operator << (std::ostream &out, const GeomCacheEntry &entry) {
  entry.output(out);
  return out;
}
