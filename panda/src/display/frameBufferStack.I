// Filename: frameBufferStack.I
// Created by:  drose (06Oct99)
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
//     Function: FrameBufferStack::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FrameBufferStack::
FrameBufferStack() {
  _stack_level = -1;
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferStack::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FrameBufferStack::
~FrameBufferStack() {
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferStack::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE FrameBufferStack::
FrameBufferStack(const FrameBufferStack &copy) :
  _frame_buffer(copy._frame_buffer),
  _stack_level(copy._stack_level)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FrameBufferStack::Copy Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void FrameBufferStack::
operator =(const FrameBufferStack &copy) {
  _frame_buffer = copy._frame_buffer;
  _stack_level = copy._stack_level;
}
