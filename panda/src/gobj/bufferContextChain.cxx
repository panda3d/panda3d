// Filename: bufferContextChain.cxx
// Created by:  drose (16Mar06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "bufferContextChain.h"
#include "bufferContext.h"
#include "indent.h"

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
    nassertv(((BufferContext *)llnode)->_owning_chain == &other);
    ((BufferContext *)llnode)->_owning_chain = this;
    llnode = ((BufferContext *)llnode)->_next;
  }

  take_list_from(&other);
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BufferContextChain::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << _count << " objects, consuming " << _total_size << " bytes:\n";

  LinkedListNode *llnode = _next;
  while (llnode != this) {
    ((BufferContext *)llnode)->write(out, indent_level + 2);
    llnode = ((BufferContext *)llnode)->_next;
  }
}
