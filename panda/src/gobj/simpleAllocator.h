// Filename: simpleAllocator.h
// Created by:  drose (12May07)
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

#ifndef SIMPLEALLOCATOR_H
#define SIMPLEALLOCATOR_H

#include "pandabase.h"
#include "linkedListNode.h"

class SimpleAllocatorBlock;

////////////////////////////////////////////////////////////////////
//       Class : SimpleAllocator
// Description : An implementation of a very simple block allocator.
//               This class can allocate ranges of nonnegative
//               integers within a specified upper limit; it uses a
//               simple first-fit algorithm to find the next available
//               space.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SimpleAllocator : public LinkedListNode {
PUBLISHED:
  INLINE SimpleAllocator(size_t max_size);
  virtual ~SimpleAllocator();

  SimpleAllocatorBlock *alloc(size_t size);

  INLINE size_t get_total_size() const;
  INLINE size_t get_max_size() const;
  INLINE void set_max_size(size_t max_size);
  INLINE size_t get_contiguous() const;

  INLINE SimpleAllocatorBlock *get_first_block() const;

  void output(ostream &out) const;
  void write(ostream &out) const;

protected:
  virtual SimpleAllocatorBlock *make_block(size_t start, size_t size);
  INLINE void mark_contiguous(const LinkedListNode *block);

private:
  // This is implemented as a linked-list chain of allocated blocks.
  // Free blocks are implicit.  Blocks are kept in sorted order from
  // beginning to end.  Allocating a block means creating a new entry
  // in the chain wherever it may fit; freeing a block means simply
  // removing the allocated block from the chain.  With this simple
  // approach, there is no need to merge adjacent free blocks to
  // straighten out fragmentation, since free blocks are not stored.
  // However, it does mean we have to walk through a list of adjacent
  // allocated blocks in order to find the free blocks.
  size_t _total_size;
  size_t _max_size;

  // This is what we currently believe our max contiguous space to be.
  // This guess might be larger than the actual available space, but
  // it will not be smaller.
  size_t _contiguous;

  friend class SimpleAllocatorBlock;
};

////////////////////////////////////////////////////////////////////
//       Class : SimpleAllocatorBlock
// Description : A single block as returned from
//               SimpleAllocator::alloc().
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SimpleAllocatorBlock : public LinkedListNode {
protected:
  INLINE SimpleAllocatorBlock(SimpleAllocator *alloc,
                              size_t start, size_t size);

PUBLISHED:
  INLINE ~SimpleAllocatorBlock();
  INLINE void free();

  INLINE SimpleAllocator *get_allocator() const;

  INLINE size_t get_start() const;
  INLINE size_t get_size() const;
  INLINE bool is_free() const;

  INLINE size_t get_max_size() const;
  INLINE bool realloc(size_t size);

  INLINE SimpleAllocatorBlock *get_next_block() const;

  void output(ostream &out) const;

private:
  SimpleAllocator *_allocator;
  size_t _start;
  size_t _size;

  friend class SimpleAllocator;
};

INLINE ostream &operator << (ostream &out, const SimpleAllocator &obj) {
  obj.output(out);
  return out;
}

INLINE ostream &operator << (ostream &out, const SimpleAllocatorBlock &obj) {
  obj.output(out);
  return out;
}

#include "simpleAllocator.I"

#endif
