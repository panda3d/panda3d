// Filename: vertexDataBlock.I
// Created by:  drose (04Jun07)
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
//     Function: VertexDataBlock::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE VertexDataBlock::
VertexDataBlock(VertexDataPage *page, size_t start, size_t size) :
  SimpleAllocatorBlock(page, start, size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::get_page
//       Access: Published
//  Description: Returns the page from which this buffer was
//               allocated.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage *VertexDataBlock::
get_page() const {
  return (VertexDataPage *)get_allocator();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::get_pointer
//       Access: Public
//  Description: Returns a pointer to the start of the allocated
//               memory for this buffer, or NULL if the data is not
//               currently resident.  If the data is not currently
//               resident, this will implicitly request it to become
//               resident soon.
//
//               If force is true, this method will never return NULL,
//               but may block until the data is available.
////////////////////////////////////////////////////////////////////
INLINE unsigned char *VertexDataBlock::
get_pointer(bool force) const {
  nassertr(get_page() != (VertexDataPage *)NULL, NULL);
  unsigned char *page_data = get_page()->get_page_data(force);
  if (page_data == (unsigned char *)NULL) {
    return NULL;
  } else {
    return page_data + get_start();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBlock::get_next_block
//       Access: Published
//  Description: Returns a pointer to the next allocated block in the
//               chain, or NULL if there are no more allocated blocks.
////////////////////////////////////////////////////////////////////
INLINE VertexDataBlock *VertexDataBlock::
get_next_block() const {
  return (VertexDataBlock *)SimpleAllocatorBlock::get_next_block();
}
