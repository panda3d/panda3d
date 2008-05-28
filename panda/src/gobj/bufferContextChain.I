// Filename: bufferContextChain.I
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


////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BufferContextChain::
BufferContextChain() :
  LinkedListNode(true),  // This object is the root of a list of BufferContexts.
  _total_size(0),
  _count(0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BufferContextChain::
~BufferContextChain() {
  nassertv(_total_size == 0 && _count == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::get_total_size
//       Access: Public
//  Description: Returns the total number of bytes represented by the
//               BufferContexts currently assigned to this tracker.
////////////////////////////////////////////////////////////////////
INLINE size_t BufferContextChain::
get_total_size() const {
  return _total_size;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::get_count
//       Access: Public
//  Description: Returns the total number of BufferContexts currently
//               assigned to this tracker.
////////////////////////////////////////////////////////////////////
INLINE int BufferContextChain::
get_count() const {
  return _count;
}

////////////////////////////////////////////////////////////////////
//     Function: BufferContextChain::adjust_bytes
//       Access: Private
//  Description: Adds or removes some from the total byte count.
////////////////////////////////////////////////////////////////////
INLINE void BufferContextChain::
adjust_bytes(int delta) {
  _total_size += delta;
}
