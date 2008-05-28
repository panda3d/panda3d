// Filename: pipelineCyclerLinks.I
// Created by:  drose (16Feb06)
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


#ifdef THREADED_PIPELINE
////////////////////////////////////////////////////////////////////
//     Function: PipelineCyclerLinks::Constructor
//       Access: Protected
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
//       Access: Protected
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
//       Access: Protected
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
//       Access: Protected
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
