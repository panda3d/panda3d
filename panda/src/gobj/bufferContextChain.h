/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bufferContextChain.h
 * @author drose
 * @date 2006-03-16
 */

#ifndef BUFFERCONTEXTCHAIN_H
#define BUFFERCONTEXTCHAIN_H

#include "pandabase.h"
#include "linkedListNode.h"

class BufferContext;

/**
 * This class maintains a linked list of BufferContexts that might be
 * allocated on the graphics card in some context.  There is a different
 * BufferContextChain for resident textures, active textures, evicted
 * textures, etc.
 *
 * The primary purpose of this class is to facilitate PStats reporting of
 * graphics memory usage.
 */
class EXPCL_PANDA_GOBJ BufferContextChain : private LinkedListNode {
public:
  INLINE BufferContextChain();
  INLINE ~BufferContextChain();

  INLINE size_t get_total_size() const;
  INLINE int get_count() const;

  BufferContext *get_first();

  void take_from(BufferContextChain &other);

  void write(std::ostream &out, int indent_level) const;

private:
  INLINE void adjust_bytes(int delta);
  size_t _total_size;
  int _count;

  friend class BufferContext;
};

#include "bufferContextChain.I"

#endif
