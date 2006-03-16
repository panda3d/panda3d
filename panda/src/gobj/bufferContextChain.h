// Filename: bufferContextChain.h
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

#ifndef BUFFERCONTEXTCHAIN_H
#define BUFFERCONTEXTCHAIN_H

#include "pandabase.h"
#include "linkedListNode.h"

class BufferContext;

////////////////////////////////////////////////////////////////////
//       Class : BufferContextChain
// Description : This class maintains a linked list of BufferContexts
//               that might be allocated on the graphics card in some
//               context.  There is a different BufferContextChain for
//               resident textures, active textures, evicted textures,
//               etc.
//
//               The primary purpose of this class is to facilitate
//               PStats reporting of graphics memory usage.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BufferContextChain : private LinkedListNode {
public:
  INLINE BufferContextChain();
  INLINE ~BufferContextChain();

  INLINE size_t get_total_size() const;
  INLINE int get_count() const;

  BufferContext *get_first();

  void take_from(BufferContextChain &other);

private:
  INLINE void adjust_bytes(int delta);
  size_t _total_size;
  int _count;

  friend class BufferContext;
};

#include "bufferContextChain.I"

#endif

