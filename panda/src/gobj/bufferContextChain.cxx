// Filename: bufferContextChain.cxx
// Created by:  drose (16Mar06)
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

#include "bufferContextChain.h"
#include "bufferContext.h"

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::get_first
//       Access: Public
//  Description: Returns the first BufferContext object stored in the
//               tracker.  You can walk through the entire list of
//               objects stored on the tracker by calling get_next()
//               on each returned object, until the return value is
//               NULL.
////////////////////////////////////////////////////////////////////
BufferContext *BufferContextChain::
get_first() {
  // This method is declared non-inline so we can include
  // bufferContext.h, which is necessary for proper downcasting of the
  // _next pointer.
  if (_next == this) {
    return NULL;
  }
  return (BufferContext *)_next;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::take_from
//       Access: Public
//  Description: Moves all of the BufferContexts from the other
//               tracker onto this one.
////////////////////////////////////////////////////////////////////
void BufferContextChain::
take_from(BufferContextChain &other) {
  _total_size += other._total_size;
  _count += other._count;
  other._total_size = 0;
  other._count = 0;

  LinkedListNode *llnode = other._next;
  while (llnode != &other) {
    ((BufferContext *)llnode)->_owning_chain = this;
    llnode = ((BufferContext *)llnode)->_next;
  }

  take_list_from(&other);
}
