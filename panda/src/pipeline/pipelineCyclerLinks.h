// Filename: pipelineCyclerLinks.h
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

#ifndef PIPELINECYCLERLINKS_H
#define PIPELINECYCLERLINKS_H

#include "pandabase.h"
#include "selectThreadImpl.h"  // for THREADED_PIPELINE definition
#include "pnotify.h"

////////////////////////////////////////////////////////////////////
//       Class : PipelineCyclerLinks
// Description : This just stores the pointers to implement a
//               doubly-linked list of PipelineCyclers for a
//               particular Pipeline object.  We use a hand-rolled
//               linked list rather than any STL container, because we
//               want PipelineCyclers to be able to add and remove
//               themselves from this list very quickly.
//
//               These pointers are inherited from this separate class
//               so the Pipeline object itself can be the root of the
//               linked list.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PIPELINE PipelineCyclerLinks {
protected:
#ifdef THREADED_PIPELINE
  INLINE PipelineCyclerLinks();
  INLINE ~PipelineCyclerLinks();

  INLINE void make_head();
  INLINE void clear_head();

  INLINE void remove_from_list();
  INLINE void insert_before(PipelineCyclerLinks *node);

  INLINE void take_list(PipelineCyclerLinks &other);

  PipelineCyclerLinks *_prev, *_next;
#endif
  friend class Pipeline;
};

#include "pipelineCyclerLinks.I"

#endif
