/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file vertexDataBlock.I
 * @author drose
 * @date 2007-06-04
 */

/**
 *
 */
INLINE VertexDataBlock::
VertexDataBlock(VertexDataPage *page, size_t start, size_t size) :
  SimpleAllocatorBlock(page, start, size)
{
}

/**
 * Returns the page from which this buffer was allocated.
 */
INLINE VertexDataPage *VertexDataBlock::
get_page() const {
  return (VertexDataPage *)get_allocator();
}

/**
 * Returns a pointer to the start of the allocated memory for this buffer, or
 * NULL if the data is not currently resident.  If the data is not currently
 * resident, this will implicitly request it to become resident soon.
 *
 * If force is true, this method will never return NULL, but may block until
 * the data is available.
 */
INLINE unsigned char *VertexDataBlock::
get_pointer(bool force) const {
  nassertr(get_page() != nullptr, nullptr);
  unsigned char *page_data = get_page()->get_page_data(force);
  if (page_data == nullptr) {
    return nullptr;
  } else {
    return page_data + get_start();
  }
}

/**
 * Returns a pointer to the next allocated block in the chain, or NULL if
 * there are no more allocated blocks.
 */
INLINE VertexDataBlock *VertexDataBlock::
get_next_block() const {
  return (VertexDataBlock *)SimpleAllocatorBlock::get_next_block();
}
