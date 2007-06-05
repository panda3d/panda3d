// Filename: vertexDataBook.cxx
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

#include "vertexDataBook.h"
#include "mutexHolder.h"

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataBook::
VertexDataBook(size_t block_size) : _block_size(block_size) {
  _next_pi = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
VertexDataBook::
~VertexDataBook() {
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::alloc
//       Access: Published
//  Description: Allocates and returns a new VertexDataBuffer of the
//               requested size.
////////////////////////////////////////////////////////////////////
VertexDataBlock *VertexDataBook::
alloc(size_t size) {
  MutexHolder holder(_lock);

  // First, try to allocate from the last page that worked; then
  // continue to the end of the list.
  size_t pi = _next_pi;
  while (pi < _pages.size()) {
    VertexDataBlock *block = _pages[pi]->alloc(size);
    if (block != (VertexDataBlock *)NULL) {
      _next_pi = pi;
      return block;
    }
    if (_pages[pi]->is_empty()) {
      // This page is empty, but must have been too small.  Create a
      // new page in its place.
      delete _pages[pi];
      _pages[pi] = create_new_page(size);
      VertexDataBlock *block = _pages[pi]->alloc(size);
      return block;
    }
    ++pi;
  }

  // Then, go back to the beginning and try those pages.
  pi = 0;
  _next_pi = min(_next_pi, _pages.size());
  while (pi < _next_pi) {
    VertexDataBlock *block = _pages[pi]->alloc(size);
    if (block != (VertexDataBlock *)NULL) {
      _next_pi = pi;
      return block;
    }
    if (_pages[pi]->is_empty()) {
      // This page is empty, but must have been too small.  Create a
      // new page in its place.
      delete _pages[pi];
      _pages[pi] = create_new_page(size);
      return _pages[pi]->alloc(size);
    }
    ++pi;
  }

  // No page was good enough.  Create a new page.  Make it at least
  // large enough to hold this requested block.
  VertexDataPage *page = create_new_page(size);
  _pages.push_back(page);
  VertexDataBlock *block = page->alloc(size);
  return block;
}

////////////////////////////////////////////////////////////////////
//     Function: VertexDataBook::save_to_disk
//       Access: Published
//  Description: Writes all pages to disk immediately, just in case
//               they get evicted later.  It makes sense to make this
//               call just before taking down a loading screen, to
//               minimize chugs from saving pages inadvertently later.
////////////////////////////////////////////////////////////////////
void VertexDataBook::
save_to_disk() {
  Pages::iterator pi;
  for (pi = _pages.begin(); pi != _pages.end(); ++pi) {
    (*pi)->save_to_disk();
  }
}

