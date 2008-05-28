// Filename: simpleAllocator.I
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


////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocator::
SimpleAllocator(size_t max_size, Mutex &lock) : 
  LinkedListNode(true),
  _total_size(0),
  _max_size(max_size),
  _contiguous(max_size),
  _lock(lock)
{
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
  MutexHolder holder(_lock);
  return do_alloc(size);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::is_empty
//       Access: Published
//  Description: Returns true if there are no blocks allocated on this
//               page, or false if there is at least one.
////////////////////////////////////////////////////////////////////
INLINE bool SimpleAllocator::
is_empty() const {
  MutexHolder holder(_lock);
  return do_is_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::get_total_size
//       Access: Published
//  Description: Returns the total size of allocated objects.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocator::
get_total_size() const {
  MutexHolder holder(_lock);
  return _total_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::get_max_size
//       Access: Published
//  Description: Returns the available space for allocated objects.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocator::
get_max_size() const {
  MutexHolder holder(_lock);
  return _max_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::set_max_size
//       Access: Published
//  Description: Changes the available space for allocated objects.
//               This will not affect any already-allocated objects,
//               but will have an effect on future calls to alloc().
////////////////////////////////////////////////////////////////////
INLINE void SimpleAllocator::
set_max_size(size_t max_size) {
  MutexHolder holder(_lock);
  _max_size = max_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::get_contiguous
//       Access: Published
//  Description: Returns an upper-bound estimate of the size of the
//               largest contiguous block that may be allocated.  It
//               is guaranteed that an attempt to allocate a block
//               larger than this will fail, though it is not
//               guaranteed that an attempt to allocate a block this
//               size or smaller will succeed.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocator::
get_contiguous() const {
  MutexHolder holder(_lock);
  return _contiguous;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::get_first_block
//       Access: Published
//  Description: Returns a pointer to the first allocated block, or
//               NULL if there are no allocated blocks.
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocatorBlock *SimpleAllocator::
get_first_block() const {
  MutexHolder holder(_lock);
  return (_next == this) ? (SimpleAllocatorBlock *)NULL : (SimpleAllocatorBlock *)_next;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::do_is_empty
//       Access: Protected
//  Description: Returns true if there are no blocks allocated on this
//               page, or false if there is at least one.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE bool SimpleAllocator::
do_is_empty() const {
  return (_next == this);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::mark_contiguous
//       Access: Protected
//  Description: Some space has been made available following the
//               indicated block.  Increase the contiguous space
//               accordingly.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE void SimpleAllocator::
mark_contiguous(const LinkedListNode *block) {
  size_t space;
  if (block == this) {
    // This is the beginning of the list.
    if (_next == this) {
      // And the list is empty.
      space = _max_size;
    } else {
      space = ((SimpleAllocatorBlock *)_next)->get_start();
    }
  } else {
    space = ((SimpleAllocatorBlock *)block)->do_get_max_size() - ((SimpleAllocatorBlock *)block)->get_size();
  }
  if (space > _contiguous) {
    _contiguous = space;
    changed_contiguous();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::Constructor
//       Access: Private
//  Description: A SimpleAllocatorBlock must be constructed via the
//               SimpleAllocator::alloc() call.
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocatorBlock::
SimpleAllocatorBlock(SimpleAllocator *alloc,
                     size_t start, size_t size) :
  _allocator(alloc),
  _start(start),
  _size(size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::Destructor
//       Access: Published
//  Description: The block automatically frees itself when it
//               destructs.
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocatorBlock::
~SimpleAllocatorBlock() {
  free();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::free
//       Access: Published
//  Description: Releases the allocated space.
////////////////////////////////////////////////////////////////////
INLINE void SimpleAllocatorBlock::
free() {
  if (_allocator != (SimpleAllocator *)NULL) {
    MutexHolder holder(_allocator->_lock);
    do_free();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::get_allocator
//       Access: Published
//  Description: Returns the SimpleAllocator object that owns this
//               block.  Returns NULL if the block has been freed.
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocator *SimpleAllocatorBlock::
get_allocator() const {
  return _allocator;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::get_start
//       Access: Published
//  Description: Returns the starting point of this block.  It is an
//               error to call this if the block has been freed.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocatorBlock::
get_start() const {
  nassertr(_allocator != (SimpleAllocator *)NULL, 0);
  return _start;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::get_size
//       Access: Published
//  Description: Returns the size of this block.  It is an
//               error to call this if the block has been freed.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocatorBlock::
get_size() const {
  nassertr(_allocator != (SimpleAllocator *)NULL, 0);
  return _size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::is_free
//       Access: Published
//  Description: Returns true if the block has been freed, false if it
//               is still valid.
////////////////////////////////////////////////////////////////////
INLINE bool SimpleAllocatorBlock::
is_free() const {
  return (_allocator != (SimpleAllocator *)NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::get_max_size
//       Access: Published
//  Description: Returns the maximum size this block can be
//               reallocated to, as limited by the following block.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocatorBlock::
get_max_size() const {
  nassertr(_allocator != (SimpleAllocator *)NULL, 0);
  MutexHolder holder(_allocator->_lock);
  return do_get_max_size();
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::realloc
//       Access: Published
//  Description: Changes the size of this block to the specified size.
//               Returns true if the change is accepted, false if
//               there was not enough room.
////////////////////////////////////////////////////////////////////
INLINE bool SimpleAllocatorBlock::
realloc(size_t size) {
  nassertr(_allocator != (SimpleAllocator *)NULL, false);
  MutexHolder holder(_allocator->_lock);
  return do_realloc(size);
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::get_next_block
//       Access: Published
//  Description: Returns a pointer to the next allocated block in the
//               chain, or NULL if there are no more allocated blocks.
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocatorBlock *SimpleAllocatorBlock::
get_next_block() const {
  nassertr(_allocator != (SimpleAllocator *)NULL, NULL);
  MutexHolder holder(_allocator->_lock);
  return (_next == _allocator) ? (SimpleAllocatorBlock *)NULL : (SimpleAllocatorBlock *)_next;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::do_free
//       Access: Protected
//  Description: Releases the allocated space.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE void SimpleAllocatorBlock::
do_free() {
  nassertv(_allocator != (SimpleAllocator *)NULL);

  _allocator->_total_size -= _size;
  LinkedListNode *prev = _prev;
  remove_from_list();
  _allocator->mark_contiguous(prev);
  _allocator = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::do_get_max_size
//       Access: Protected
//  Description: Returns the maximum size this block can be
//               reallocated to, as limited by the following block.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocatorBlock::
do_get_max_size() const {
  size_t end;
  if (_next == _allocator) {
    end = _allocator->_max_size;
  } else {
    end = ((SimpleAllocatorBlock *)_next)->_start;
  }
  return end - _start;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocatorBlock::do_realloc
//       Access: Protected
//  Description: Changes the size of this block to the specified size.
//               Returns true if the change is accepted, false if
//               there was not enough room.
//
//               Assumes the lock is already held.
////////////////////////////////////////////////////////////////////
INLINE bool SimpleAllocatorBlock::
do_realloc(size_t size) {
  if (size > do_get_max_size()) {
    return false;
  }

  _allocator->_total_size -= _size;
  _allocator->_total_size += size;

  if (size < _size) {
    // We're decreasing the block size.
    _size = size;
    _allocator->mark_contiguous(this);
  } else {
    // We're increasing the block size.
    _size = size;
  }
  return true;
}
