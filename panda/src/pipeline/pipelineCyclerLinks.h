// Filename: pipelineCyclerLinks.h
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
class EXPCL_PANDA PipelineCyclerLinks {
protected:
#ifdef THREADED_PIPELINE
  INLINE PipelineCyclerLinks();
  INLINE ~PipelineCyclerLinks();

  INLINE void remove_from_list();
  INLINE void insert_before(PipelineCyclerLinks *node);

  PipelineCyclerLinks *_prev, *_next;
#endif
  friend class Pipeline;
};

#include "pipelineCyclerLinks.I"

#endif
