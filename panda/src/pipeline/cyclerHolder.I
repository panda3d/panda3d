// Filename: cyclerHolder.I
// Created by:  drose (09Feb06)
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
//     Function: CyclerHolder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CyclerHolder::
CyclerHolder(PipelineCyclerBase &cycler) {
#ifdef DO_PIPELINING
  _cycler = &cycler;
  _cycler->acquire();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CyclerHolder::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CyclerHolder::
~CyclerHolder() {
#ifdef DO_PIPELINING
  _cycler->release();
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: CyclerHolder::Copy Constructor
//       Access: Private
//  Description: Do not attempt to copy CyclerHolders.
////////////////////////////////////////////////////////////////////
INLINE CyclerHolder::
CyclerHolder(const CyclerHolder &copy) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: CyclerHolder::Copy Assignment Operator
//       Access: Private
//  Description: Do not attempt to copy CyclerHolders.
////////////////////////////////////////////////////////////////////
INLINE void CyclerHolder::
operator = (const CyclerHolder &copy) {
  nassertv(false);
}
