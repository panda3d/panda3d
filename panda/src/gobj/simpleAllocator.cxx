/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleAllocator.cxx
 * @author drose
 * @date 2007-05-12
 */

#include "simpleAllocator.h"

/**
 * Move constructor.
 */
SimpleAllocator::
SimpleAllocator(SimpleAllocator &&from) noexcept :
  LinkedListNode(std::move(from)),
  _total_size(from._total_size),
  _max_size(from._max_size),
  _contiguous(from._contiguous),
  _lock(from._lock)
{
  MutexHolder holder(_lock);
  from._total_size = 0;
  from._max_size = 0;
  from._contiguous = 0;

  // We still need to leave the list in a valid state.
  from._prev = &from;
  from._next = &from;

  // Change all the blocks to point to the new allocator.
  LinkedListNode *next = _next;
  while (next != this) {
    SimpleAllocatorBlock *block = (SimpleAllocatorBlock *)next;
    nassertv(block->_allocator == &from);
    block->_allocator = this;

    next = block->_next;
  }
}

/**
 *
 */
SimpleAllocator::
~SimpleAllocator() {
  // We're shutting down.  Force-free everything remaining.
  if (_next != (LinkedListNode *)this) {
    MutexHolder holder(_lock);
    while (_next != (LinkedListNode *)this) {
      nassertv(_next != nullptr);
      ((SimpleAllocatorBlock *)_next)->do_free();
    }
  }
}

/**
 *
 */
void SimpleAllocator::
output(std::ostream &out) const {
  MutexHolder holder(_lock);
  out << "SimpleAllocator, " << _total_size << " of " << _max_size
      << " allocated";
}

/**
 *
 */
void SimpleAllocator::
write(std::ostream &out) const {
  MutexHolder holder(_lock);
  out << "SimpleAllocator, " << _total_size << " of " << _max_size
      << " allocated";

  SimpleAllocatorBlock *block = (SimpleAllocatorBlock *)_next;
  while (block->_next != this) {
    SimpleAllocatorBlock *next = (SimpleAllocatorBlock *)block->_next;

    out << "  " << *block << "\n";
    block = next;
  }
}

/**
 * Allocates a new block.  Returns NULL if a block of the requested size
 * cannot be allocated.
 *
 * To free the allocated block, call block->free(), or simply delete the block
 * pointer.
 *
 * Assumes the lock is already held.
 */
SimpleAllocatorBlock *SimpleAllocator::
do_alloc(size_t size, size_t alignment) {
  if (size > _contiguous) {
    // Don't even bother.
    return nullptr;
  }

  // First fit algorithm: walk through all the empty blocks until we find one
  // that has enough room.

  SimpleAllocatorBlock *block = nullptr;
  size_t end = 0;
  size_t best = 0;
  if (_next != this) {
    // We have at least one allocated block.
    block = (SimpleAllocatorBlock *)_next;
    end = block->_start + block->_size;

    // Scan until we have reached the last allocated block.
    while (block->_next != this) {
      SimpleAllocatorBlock *next = (SimpleAllocatorBlock *)block->_next;
      size_t start = end + ((alignment - end) % alignment);
      if (start + size <= next->_start) {
        SimpleAllocatorBlock *new_block = make_block(start, size);
        nassertr(new_block->get_allocator() == this, nullptr);

        new_block->insert_before(next);
        _total_size += size;

        if (_max_size - _total_size < _contiguous) {
          // Since we only have (_max_size - _total_size) bytes remaining, it
          // follows that our largest contiguous block must be no larger than
          // this.
          _contiguous = _max_size - _total_size;
          changed_contiguous();
        }
        return new_block;
      }
      size_t free_size = next->_start - end;
      if (free_size > best) {
        best = free_size;
      }

      block = next;
      end = block->_start + block->_size;
    }
  }

  // No free blocks; check for room at the end.
  size_t start = end + ((alignment - end) % alignment);
  if (start + size <= _max_size) {
    SimpleAllocatorBlock *new_block = make_block(start, size);
    nassertr(new_block->get_allocator() == this, nullptr);

    new_block->insert_before(this);
    _total_size += size;

    if (_max_size - _total_size < _contiguous) {
      // Since we only have (_max_size - _total_size) bytes remaining, it
      // follows that our largest contiguous block must be no larger than
      // this.
      _contiguous = _max_size - _total_size;
      changed_contiguous();
    }
    return new_block;
  }

  size_t free_size = _max_size - end;
  if (free_size > best) {
    best = free_size;
  }

  // Now that we've walked through the entire list of blocks, we really do
  // know accurately what the largest contiguous block is.
  if (_contiguous != best) {
    _contiguous = best;
    changed_contiguous();
  }

  // No room for this block.
  return nullptr;
}

/**
 * Creates a new SimpleAllocatorBlock object.  Override this function to
 * specialize the block type returned.
 */
SimpleAllocatorBlock *SimpleAllocator::
make_block(size_t start, size_t size) {
  return new SimpleAllocatorBlock(this, start, size);
}

/**
 * This callback function is made whenever the estimate of contiguous
 * available space changes, either through an alloc or free.  The lock will be
 * held.
 */
void SimpleAllocator::
changed_contiguous() {
}

/**
 *
 */
void SimpleAllocatorBlock::
output(std::ostream &out) const {
  if (_allocator == nullptr) {
    out << "free block\n";
  } else {
    MutexHolder holder(_allocator->_lock);
    out << "block of size " << _size << " at " << _start;
  }
}
