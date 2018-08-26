/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file simpleAllocator.I
 * @author drose
 * @date 2007-05-12
 */

/**
 *
 */
INLINE SimpleAllocator::
SimpleAllocator(size_t max_size, Mutex &lock) :
  LinkedListNode(true),
  _total_size(0),
  _max_size(max_size),
  _contiguous(max_size),
  _lock(lock)
{
}

/**
 * Allocates a new block.  Returns NULL if a block of the requested size
 * cannot be allocated.
 *
 * To free the allocated block, call block->free(), or simply delete the block
 * pointer.
 */
SimpleAllocatorBlock *SimpleAllocator::
alloc(size_t size, size_t alignment) {
  MutexHolder holder(_lock);
  return do_alloc(size, alignment);
}

/**
 * Returns true if there are no blocks allocated on this page, or false if
 * there is at least one.
 */
INLINE bool SimpleAllocator::
is_empty() const {
  MutexHolder holder(_lock);
  return do_is_empty();
}

/**
 * Returns the total size of allocated objects.
 */
INLINE size_t SimpleAllocator::
get_total_size() const {
  MutexHolder holder(_lock);
  return _total_size;
}

/**
 * Returns the available space for allocated objects.
 */
INLINE size_t SimpleAllocator::
get_max_size() const {
  MutexHolder holder(_lock);
  return _max_size;
}

/**
 * Changes the available space for allocated objects.  This will not affect
 * any already-allocated objects, but will have an effect on future calls to
 * alloc().
 */
INLINE void SimpleAllocator::
set_max_size(size_t max_size) {
  MutexHolder holder(_lock);
  _max_size = max_size;
}

/**
 * Returns an upper-bound estimate of the size of the largest contiguous block
 * that may be allocated.  It is guaranteed that an attempt to allocate a
 * block larger than this will fail, though it is not guaranteed that an
 * attempt to allocate a block this size or smaller will succeed.
 */
INLINE size_t SimpleAllocator::
get_contiguous() const {
  MutexHolder holder(_lock);
  return _contiguous;
}

/**
 * Returns a pointer to the first allocated block, or NULL if there are no
 * allocated blocks.
 */
INLINE SimpleAllocatorBlock *SimpleAllocator::
get_first_block() const {
  MutexHolder holder(_lock);
  return (_next == this) ? nullptr : (SimpleAllocatorBlock *)_next;
}

/**
 * Returns true if there are no blocks allocated on this page, or false if
 * there is at least one.
 *
 * Assumes the lock is already held.
 */
INLINE bool SimpleAllocator::
do_is_empty() const {
  return (_next == this);
}

/**
 * Some space has been made available following the indicated block.  Increase
 * the contiguous space accordingly.
 *
 * Assumes the lock is already held.
 */
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

/**
 * A SimpleAllocatorBlock must be constructed via the SimpleAllocator::alloc()
 * call.
 */
INLINE SimpleAllocatorBlock::
SimpleAllocatorBlock(SimpleAllocator *alloc,
                     size_t start, size_t size) :
  _allocator(alloc),
  _start(start),
  _size(size)
{
}

/**
 * Transfers ownership from the given SimpleAllocatorBlock to this one.
 */
INLINE SimpleAllocatorBlock::
SimpleAllocatorBlock(SimpleAllocatorBlock &&from) :
  _allocator(from._allocator)
{
  if (_allocator == nullptr) {
    return;
  }

  MutexHolder holder(_allocator->_lock);
  _start = from._start;
  _size = from._size;
  LinkedListNode::operator = (std::move(from));
  from._allocator = nullptr;
}

/**
 * The block automatically frees itself when it destructs.
 */
INLINE SimpleAllocatorBlock::
~SimpleAllocatorBlock() {
  free();
}

/**
 * Frees this block and instead takes ownership of the given other block.
 */
INLINE SimpleAllocatorBlock &SimpleAllocatorBlock::
operator = (SimpleAllocatorBlock &&from) {
  free();

  _allocator = from._allocator;
  if (_allocator == nullptr) {
    _start = 0;
    _size = 0;
    return *this;
  }

  MutexHolder holder(_allocator->_lock);
  _start = from._start;
  _size = from._size;
  LinkedListNode::operator = (std::move(from));
  from._allocator = nullptr;
  return *this;
}

/**
 * Releases the allocated space.
 */
INLINE void SimpleAllocatorBlock::
free() {
  if (_allocator != nullptr) {
    MutexHolder holder(_allocator->_lock);
    do_free();
  }
}

/**
 * Returns the SimpleAllocator object that owns this block.  Returns NULL if
 * the block has been freed.
 */
INLINE SimpleAllocator *SimpleAllocatorBlock::
get_allocator() const {
  return _allocator;
}

/**
 * Returns the starting point of this block.  It is an error to call this if
 * the block has been freed.
 */
INLINE size_t SimpleAllocatorBlock::
get_start() const {
  nassertr(_allocator != nullptr, 0);
  return _start;
}

/**
 * Returns the size of this block.  It is an error to call this if the block
 * has been freed.
 */
INLINE size_t SimpleAllocatorBlock::
get_size() const {
  nassertr(_allocator != nullptr, 0);
  return _size;
}

/**
 * Returns true if the block has been freed, false if it is still valid.
 */
INLINE bool SimpleAllocatorBlock::
is_free() const {
  return (_allocator != nullptr);
}

/**
 * Returns the maximum size this block can be reallocated to, as limited by
 * the following block.
 */
INLINE size_t SimpleAllocatorBlock::
get_max_size() const {
  nassertr(_allocator != nullptr, 0);
  MutexHolder holder(_allocator->_lock);
  return do_get_max_size();
}

/**
 * Changes the size of this block to the specified size.  Returns true if the
 * change is accepted, false if there was not enough room.
 */
INLINE bool SimpleAllocatorBlock::
realloc(size_t size) {
  nassertr(_allocator != nullptr, false);
  MutexHolder holder(_allocator->_lock);
  return do_realloc(size);
}

/**
 * Returns a pointer to the next allocated block in the chain, or NULL if
 * there are no more allocated blocks.
 */
INLINE SimpleAllocatorBlock *SimpleAllocatorBlock::
get_next_block() const {
  nassertr(_allocator != nullptr, nullptr);
  MutexHolder holder(_allocator->_lock);
  return (_next == _allocator) ? nullptr : (SimpleAllocatorBlock *)_next;
}

/**
 * Releases the allocated space.
 *
 * Assumes the lock is already held.
 */
INLINE void SimpleAllocatorBlock::
do_free() {
  nassertv(_allocator != nullptr);

  _allocator->_total_size -= _size;
  LinkedListNode *prev = _prev;
  remove_from_list();
  _allocator->mark_contiguous(prev);
  _allocator = nullptr;
}

/**
 * Returns the maximum size this block can be reallocated to, as limited by
 * the following block.
 *
 * Assumes the lock is already held.
 */
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

/**
 * Changes the size of this block to the specified size.  Returns true if the
 * change is accepted, false if there was not enough room.
 *
 * Assumes the lock is already held.
 */
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
