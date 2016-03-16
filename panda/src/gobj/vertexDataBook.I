/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataBook.I
 * @author drose
 * @date 2007-05-16
 */

/**
 * Allocates and returns a new VertexDataBuffer of the requested size.
 */
INLINE VertexDataBlock *VertexDataBook::
alloc(size_t size) {
  MutexHolder holder(_lock);
  return do_alloc(size);
}

/**
 * Returns the number of pages created for the book.
 */
INLINE size_t VertexDataBook::
get_num_pages() const {
  return _pages.size();
}

/**
 * Creates a new page of sufficient size to hold the requested block.  The
 * page is not added to the _pages list.
 */
INLINE VertexDataPage *VertexDataBook::
create_new_page(size_t size) {
  size_t page_size = ((size + _block_size - 1) / _block_size) * _block_size;
  return new VertexDataPage(this, page_size, _block_size);
}
