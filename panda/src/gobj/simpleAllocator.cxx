// Filename: simpleAllocator.cxx
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

#include "simpleAllocator.h"

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::Destructor
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SimpleAllocator::
~SimpleAllocator() {
  // We're shutting down.  Force-free everything remaining.
  while (_next != (LinkedListNode *)this) {
    nassertv(_next != (LinkedListNode *)NULL);
    ((SimpleAllocatorBlock *)_next)->free();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::alloc
//       Access: Published
//  Description: Allocates a new block.  Returns NULL if a block of the
//               requested size cannot be allocated.
//
//               To free the allocated block, call block->free(), or
//               simply delete the block pointer.
////////////////////////////////////////////////////////////////////
SimpleAllocatorBlock *SimpleAllocator::
alloc(size_t size) {
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
        new_block->insert_before(next);
        _total_size += size;
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
    new_block->insert_before(this);
    _total_size += size;
    return new_block;
  }

  if (free_size > best) {
    best = free_size;
  }

  // Now that we've walked through the entire list of blocks, we
  // really do know accurately what the largest contiguous block is.
  _contiguous = best;

  // No room for this block.
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SimpleAllocator::
output(ostream &out) const {
  out << "SimpleAllocator, " << _total_size << " of " << _max_size 
      << " allocated";
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
    out << "block of size " << _size << " at " << _start;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void SimpleAllocator::
write(ostream &out) const {
  out << *this << ":\n";

  SimpleAllocatorBlock *block = (SimpleAllocatorBlock *)_next;
  while (block->_next != this) {
    SimpleAllocatorBlock *next = (SimpleAllocatorBlock *)block->_next;

    out << "  " << *block << "\n";
    block = next;
  }
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

