// Filename: vertexDataBook.I
// Created by:  drose (16May07)
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
//     Function: VertexDataBook::get_num_pages
//       Access: Published
//  Description: Returns the number of pages created for the book.
////////////////////////////////////////////////////////////////////
INLINE int VertexDataBook::
get_num_pages() const {
  return _pages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::get_page
//       Access: Published
//  Description: Returns the nth page created for the book.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage *VertexDataBook::
get_page(int n) const {
  nassertr(n >= 0 && n < (int)_pages.size(), NULL);
  return _pages[n];
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::create_new_page
//       Access: Private
//  Description: Creates a new page of sufficient size to hold the
//               requested block.  The page is not added to the _pages
//               list.
////////////////////////////////////////////////////////////////////
INLINE VertexDataPage *VertexDataBook::
create_new_page(size_t size) {
  size_t page_size = ((size + _block_size - 1) / _block_size) * _block_size;
  return new VertexDataPage(page_size);
}
