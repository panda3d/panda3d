/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file deletedBufferChain.cxx
 * @author drose
 * @date 2007-07-20
 */

#include "deletedBufferChain.h"
#include "memoryHook.h"

/**
 * Use the global MemoryHook to get a new DeletedBufferChain of the
 * appropriate size.
 */
DeletedBufferChain::
DeletedBufferChain(size_t buffer_size) {
  _deleted_chain = nullptr;
  _buffer_size = buffer_size;

  // We must allocate at least this much space for bookkeeping reasons.
  _buffer_size = std::max(_buffer_size, sizeof(ObjectNode));
}

/**
 * Allocates the memory for a new buffer of the indicated size (which must be
 * no greater than the fixed size associated with the DeletedBufferChain).
 */
void *DeletedBufferChain::
allocate(size_t size, TypeHandle type_handle) {
#ifdef USE_DELETED_CHAIN
  // TAU_PROFILE("void *DeletedBufferChain::allocate(size_t, TypeHandle)", "
  // ", TAU_USER);
  assert(size <= _buffer_size);

  // Determine how much space to allocate.
  const size_t alloc_size = _buffer_size + flag_reserved_bytes + MEMORY_HOOK_ALIGNMENT - 1;

  ObjectNode *obj;

  _lock.lock();
  if (_deleted_chain != nullptr) {
    obj = _deleted_chain;
    _deleted_chain = _deleted_chain->_next;
    _lock.unlock();

#ifdef USE_DELETEDCHAINFLAG
    assert(obj->_flag == (AtomicAdjust::Integer)DCF_deleted);
    obj->_flag = DCF_alive;
#endif  // USE_DELETEDCHAINFLAG

    void *ptr = node_to_buffer(obj);

#ifdef DO_MEMORY_USAGE
    // type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_inactive,
    // alloc_size);
    type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
#endif  // DO_MEMORY_USAGE

    return ptr;
  }
  _lock.unlock();

  // If we get here, the deleted_chain is empty; we have to allocate a new
  // object from the system pool.

  // Allocate memory, and make sure the object starts at the proper alignment.
  void *mem = NeverFreeMemory::alloc(alloc_size);
  uintptr_t aligned = ((uintptr_t)mem + flag_reserved_bytes + MEMORY_HOOK_ALIGNMENT - 1) & ~(MEMORY_HOOK_ALIGNMENT - 1);
  obj = (ObjectNode *)(aligned - flag_reserved_bytes);

#ifdef USE_DELETEDCHAINFLAG
  obj->_flag = DCF_alive;
#endif  // USE_DELETEDCHAINFLAG

  void *ptr = node_to_buffer(obj);

#ifndef NDEBUG
  assert(((uintptr_t)ptr % MEMORY_HOOK_ALIGNMENT) == 0);
#endif

#ifdef DO_MEMORY_USAGE
  type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
#endif  // DO_MEMORY_USAGE

  return ptr;

#else  // USE_DELETED_CHAIN
  return PANDA_MALLOC_SINGLE(_buffer_size);
#endif  // USE_DELETED_CHAIN
}

/**
 * Frees the memory for a buffer previously allocated via allocate().
 */
void DeletedBufferChain::
deallocate(void *ptr, TypeHandle type_handle) {
#ifdef USE_DELETED_CHAIN
  // TAU_PROFILE("void DeletedBufferChain::deallocate(void *, TypeHandle)", "
  // ", TAU_USER);
  assert(ptr != nullptr);

#ifdef DO_MEMORY_USAGE
  const size_t alloc_size = _buffer_size + flag_reserved_bytes + MEMORY_HOOK_ALIGNMENT - 1;
  type_handle.dec_memory_usage(TypeHandle::MC_deleted_chain_active, alloc_size);
  // type_handle.inc_memory_usage(TypeHandle::MC_deleted_chain_inactive,
  // _buffer_size + flag_reserved_bytes);

#endif  // DO_MEMORY_USAGE

  ObjectNode *obj = buffer_to_node(ptr);

#ifdef USE_DELETEDCHAINFLAG
  AtomicAdjust::Integer orig_flag = AtomicAdjust::compare_and_exchange(obj->_flag, DCF_alive, DCF_deleted);

  // If this assertion is triggered, you double-deleted an object.
  assert(orig_flag != (AtomicAdjust::Integer)DCF_deleted);

  // If this assertion is triggered, you tried to delete an object that was
  // never allocated, or you have heap corruption.
  assert(orig_flag == (AtomicAdjust::Integer)DCF_alive);
#endif  // USE_DELETEDCHAINFLAG

  _lock.lock();

  obj->_next = _deleted_chain;
  _deleted_chain = obj;

  _lock.unlock();

#else  // USE_DELETED_CHAIN
  PANDA_FREE_SINGLE(ptr);
#endif  // USE_DELETED_CHAIN
}
