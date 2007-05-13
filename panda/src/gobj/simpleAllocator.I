// Filename: simpleAllocator.I
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


////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SimpleAllocator::
SimpleAllocator(size_t max_size) : 
  LinkedListNode(true),
  _total_size(0),
  _max_size(max_size)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::get_total_size
//       Access: Published
//  Description: Returns the total size of allocated objects.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocator::
get_total_size() const {
  return _total_size;
}

////////////////////////////////////////////////////////////////////
//     Function: SimpleAllocator::get_max_size
//       Access: Published
//  Description: Returns the available space for allocated objects.
////////////////////////////////////////////////////////////////////
INLINE size_t SimpleAllocator::
get_max_size() const {
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
  _max_size = max_size;
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
    _allocator->_total_size -= _size;
    remove_from_list();
    _allocator = NULL;
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
