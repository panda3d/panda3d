// Filename: simpleAllocator.cxx
// Created by:  drose (12May07)
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

#include "simpleAllocator.h"

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SimpleAllocator::
~SimpleAllocator() {
  // We're shutting down.  Force-free everything remaining.
  if (_next != (LinkedListNode *)this) {
    MutexHolder holder(_lock);
    while (_next != (LinkedListNode *)this) {
      nassertv(_next != (LinkedListNode *)NULL);
      ((SimpleAllocatorBlock *)_next)->do_free();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SimpleAllocator::
output(ostream &out) const {
  MutexHolder holder(_lock);
  out << "SimpleAllocator, " << _total_size << " of " << _max_size 
      << " allocated";
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SimpleAllocator::
write(ostream &out) const {
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

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::do_alloc
//       Access: Protected
//  Description: Allocates a new block.  Returns NULL if a block of the
//               requested size cannot be allocated.
//
//               To free the allocated block, call block->free(), or
//               simply delete the block pointer.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
SimpleAllocatorBlock *SimpleAllocator::
do_alloc(size_t size) {
  if (size > _contiguous) {
    // Don't even bother.
    return NULL;
  }

  // First fit algorithm: walk through all the empty blocks until we
  // find one that has enough room.

  SimpleAllocatorBlock *block = NULL;
  size_t end = 0;
  size_t best = 0;
  if (_next != this) {
    // We have at least one allocated block.
    block = (SimpleAllocatorBlock *)_next;
    end = block->_start + block->_size;

    // Scan until we have reached the last allocated block.
    while (block->_next != this) {
      SimpleAllocatorBlock *next = (SimpleAllocatorBlock *)block->_next;
      size_t free_size = next->_start - end;
      if (size <= free_size) {
        SimpleAllocatorBlock *new_block = make_block(end, size);
        nassertr(new_block->get_allocator() == this, NULL);

        new_block->insert_before(next);
        _total_size += size;

        if (_max_size - _total_size < _contiguous) {
          // Since we only have (_max_size - _total_size) bytes
          // remaining, it follows that our largest contiguous block
          // must be no larger than this.
          _contiguous = _max_size - _total_size;
          changed_contiguous();
        }
        return new_block;
      }
      if (free_size > best) {
        best = free_size;
      }
      
      block = next;
      end = block->_start + block->_size;
    }
  }

  // No free blocks; check for room at the end.
  size_t free_size = _max_size - end;
  if (size <= free_size) {
    SimpleAllocatorBlock *new_block = make_block(end, size);
    nassertr(new_block->get_allocator() == this, NULL);

    new_block->insert_before(this);
    _total_size += size;

    if (_max_size - _total_size < _contiguous) {
      // Since we only have (_max_size - _total_size) bytes
      // remaining, it follows that our largest contiguous block
      // must be no larger than this.
      _contiguous = _max_size - _total_size;
      changed_contiguous();
    }
    return new_block;
  }

  if (free_size > best) {
    best = free_size;
  }

  // Now that we've walked through the entire list of blocks, we
  // really do know accurately what the largest contiguous block is.
  if (_contiguous != best) {
    _contiguous = best;
    changed_contiguous();
  }

  // No room for this block.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::make_block
//       Access: Protected, Virtual
//  Description: Creates a new SimpleAllocatorBlock object.  Override
//               this function to specialize the block type returned.
////////////////////////////////////////////////////////////////////
SimpleAllocatorBlock *SimpleAllocator::
make_block(size_t start, size_t size) {
  return new SimpleAllocatorBlock(this, start, size);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::changed_contiguous
//       Access: Protected, Virtual
//  Description: This callback function is made whenever the estimate
//               of contiguous available space changes, either through
//               an alloc or free.  The lock will be held.
////////////////////////////////////////////////////////////////////
void SimpleAllocator::
changed_contiguous() {
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SimpleAllocatorBlock::
output(ostream &out) const {
  if (_allocator == (SimpleAllocator *)NULL) {
    out << "free block\n";
  } else {
    MutexHolder holder(_allocator->_lock);
    out << "block of size " << _size << " at " << _start;
  }
}
