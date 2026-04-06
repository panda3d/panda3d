/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleAllocator.h
 * @author drose
 * @date 2007-05-12
 */

#ifndef SIMPLEALLOCATOR_H
#define SIMPLEALLOCATOR_H

#include "pandabase.h"
#include "linkedListNode.h"
#include "pmutex.h"
#include "mutexHolder.h"

class SimpleAllocatorBlock;

/**
 * An implementation of a very simple block allocator.  This class can
 * allocate ranges of nonnegative integers within a specified upper limit; it
 * uses a simple first-fit algorithm to find the next available space.
 */
class EXPCL_PANDA_GOBJ SimpleAllocator : public LinkedListNode {
PUBLISHED:
  INLINE explicit SimpleAllocator(size_t max_size, Mutex &lock);
  SimpleAllocator(SimpleAllocator &&from) noexcept;
  virtual ~SimpleAllocator();

  INLINE SimpleAllocatorBlock *alloc(size_t size, size_t alignment=1);

  INLINE bool is_empty() const;
  INLINE size_t get_total_size() const;
  INLINE size_t get_max_size() const;
  INLINE void set_max_size(size_t max_size);
  INLINE size_t get_contiguous() const;

  INLINE SimpleAllocatorBlock *get_first_block() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

protected:
  SimpleAllocatorBlock *do_alloc(size_t size, size_t alignment=1);
  INLINE bool do_is_empty() const;

  virtual SimpleAllocatorBlock *make_block(size_t start, size_t size);
  INLINE void mark_contiguous(const LinkedListNode *block);
  virtual void changed_contiguous();

protected:
/*
 * This is implemented as a linked-list chain of allocated blocks.  Free
 * blocks are implicit.  Blocks are kept in sorted order from beginning to
 * end.  Allocating a block means creating a new entry in the chain wherever
 * it may fit; freeing a block means simply removing the allocated block from
 * the chain.  With this simple approach, there is no need to merge adjacent
 * free blocks to straighten out fragmentation, since free blocks are not
 * stored.  However, it does mean we have to walk through a list of adjacent
 * allocated blocks in order to find the free blocks.
 */
  size_t _total_size;
  size_t _max_size;

  // This is what we currently believe our max contiguous space to be.  This
  // guess might be larger than the actual available space, but it will not be
  // smaller.
  size_t _contiguous;

  // This mutex protects all operations within this class.  The caller must
  // pass the reference to a mutex in to the constructor, and the caller
  // remains responsible for owning the mutex.  This allows the mutex to be
  // shared where appropriate.

  // A derived class may also use it to protect itself as well, but take care
  // to call do_alloc() instead of alloc() etc.  as necessary.
  Mutex &_lock;

  friend class SimpleAllocatorBlock;
};

/**
 * A single block as returned from SimpleAllocator::alloc().
 */
class EXPCL_PANDA_GOBJ SimpleAllocatorBlock : public LinkedListNode {
protected:
  INLINE SimpleAllocatorBlock(SimpleAllocator *alloc,
                              size_t start, size_t size);

public:
  SimpleAllocatorBlock() = default;
  SimpleAllocatorBlock(const SimpleAllocatorBlock &copy) = delete;
  INLINE SimpleAllocatorBlock(SimpleAllocatorBlock &&from);

  SimpleAllocatorBlock &operator = (const SimpleAllocatorBlock &copy) = delete;
  INLINE SimpleAllocatorBlock &operator = (SimpleAllocatorBlock &&from);

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

  void output(std::ostream &out) const;

protected:
  INLINE void do_free();
  INLINE size_t do_get_max_size() const;
  INLINE bool do_realloc(size_t size);

private:
  SimpleAllocator *_allocator = nullptr;
  size_t _start = 0;
  size_t _size = 0;

  friend class SimpleAllocator;
};

INLINE std::ostream &operator << (std::ostream &out, const SimpleAllocator &obj) {
  obj.output(out);
  return out;
}

INLINE std::ostream &operator << (std::ostream &out, const SimpleAllocatorBlock &obj) {
  obj.output(out);
  return out;
}

#include "simpleAllocator.I"

#endif
