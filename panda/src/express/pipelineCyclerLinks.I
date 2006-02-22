// Filename: pipelineCyclerLinks.I
// Created by:  drose (16Feb06)
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


#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerLinks::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerLinks::
PipelineCyclerLinks() {
#ifndef NDEBUG
  _next = NULL;
  _prev = NULL;
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerLinks::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PipelineCyclerLinks::
~PipelineCyclerLinks() {
  nassertv(_next == NULL && _prev == NULL);
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerLinks::remove_from_list
//       Access: Private
//  Description: Removes a PipelineCyclerLinks record from the
//               doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerLinks::
remove_from_list() {
  nassertv(_prev->_next == this && _next->_prev == this);
  _prev->_next = _next;
  _next->_prev = _prev;
#ifndef NDEBUG
  _next = NULL;
  _prev = NULL;
#endif
}
#endif  // THREADED_PIPELINE

#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerLinks::insert_before
//       Access: Private
//  Description: Adds a PipelineCyclerLinks record before the indicated
//               node in the doubly-linked list.
////////////////////////////////////////////////////////////////////
INLINE void PipelineCyclerLinks::
insert_before(PipelineCyclerLinks *node) {
  nassertv(node->_prev->_next == node && node->_next->_prev == node);
  nassertv(_prev == (PipelineCyclerLinks *)NULL &&
           _next == (PipelineCyclerLinks *)NULL);
  _prev = node->_prev;
  _next = node;
  _prev->_next = this;
  node->_prev = this;
}
#endif  // THREADED_PIPELINE
