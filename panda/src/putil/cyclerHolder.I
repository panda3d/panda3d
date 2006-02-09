// Filename: cyclerHolder.I
// Created by:  drose (09Feb06)
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
//     Function: CyclerHolder::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE CyclerHolder::
CyclerHolder(PipelineCyclerBase &cycler) {
#ifdef DO_PIPELINING
  _cycler = &cycler;
  _cycler->lock();
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
