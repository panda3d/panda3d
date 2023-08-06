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

#include <set>

DeletedBufferChain DeletedBufferChain::_small_deleted_chains[DeletedBufferChain::num_small_deleted_chains] = {
  DeletedBufferChain(sizeof(void *)),
  DeletedBufferChain(sizeof(void *) * 2),
  DeletedBufferChain(sizeof(void *) * 3),
  DeletedBufferChain(sizeof(void *) * 4),
  DeletedBufferChain(sizeof(void *) * 5),
  DeletedBufferChain(sizeof(void *) * 6),
  DeletedBufferChain(sizeof(void *) * 7),
  DeletedBufferChain(sizeof(void *) * 8),
  DeletedBufferChain(sizeof(void *) * 9),
  DeletedBufferChain(sizeof(void *) * 10),
  DeletedBufferChain(sizeof(void *) * 11),
  DeletedBufferChain(sizeof(void *) * 12),
  DeletedBufferChain(sizeof(void *) * 13),
  DeletedBufferChain(sizeof(void *) * 14),
  DeletedBufferChain(sizeof(void *) * 15),
  DeletedBufferChain(sizeof(void *) * 16),
  DeletedBufferChain(sizeof(void *) * 17),
  DeletedBufferChain(sizeof(void *) * 18),
  DeletedBufferChain(sizeof(void *) * 19),
  DeletedBufferChain(sizeof(void *) * 20),
  DeletedBufferChain(sizeof(void *) * 21),
  DeletedBufferChain(sizeof(void *) * 22),
  DeletedBufferChain(sizeof(void *) * 23),
  DeletedBufferChain(sizeof(void *) * 24),
};

/**
 * Allocates the memory for a new buffer of the indicated size (which must be
 * no greater than the fixed size associated with the DeletedBufferChain).
 */
void *DeletedBufferChain::
allocate(size_t size, TypeHandle type_handle) {
#ifdef USE_DELETED_CHAIN
  // TAU_PROFILE("void *DeletedBufferChain::allocate(size_t, TypeHandle)", "
  // ", TAU_USER);
  // If this triggers, maybe you forgot ALLOC_DELETED_CHAIN in a subclass?
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
    DeletedChainFlag orig_flag = obj->_flag.exchange(DCF_alive, std::memory_order_relaxed);
    assert(orig_flag == DCF_deleted);
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
  obj->_flag.store(DCF_alive, std::memory_order_relaxed);
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
  DeletedChainFlag orig_flag = DCF_alive;
  if (UNLIKELY(!obj->_flag.compare_exchange_strong(orig_flag, DCF_deleted,
                                                   std::memory_order_relaxed))) {
    // If this assertion is triggered, you double-deleted an object.
    assert(orig_flag != DCF_deleted);

    // If this assertion is triggered, you tried to delete an object that was
    // never allocated, or you have heap corruption.
    assert(orig_flag == DCF_alive);
  }
#endif  // USE_DELETEDCHAINFLAG

  _lock.lock();

  obj->_next = _deleted_chain;
  _deleted_chain = obj;

  _lock.unlock();

#else  // USE_DELETED_CHAIN
  PANDA_FREE_SINGLE(ptr);
#endif  // USE_DELETED_CHAIN
}

/**
 * Returns a new DeletedBufferChain.
 */
DeletedBufferChain *DeletedBufferChain::
get_large_deleted_chain(size_t buffer_size) {
  static MutexImpl lock;
  lock.lock();
  static std::set<DeletedBufferChain> deleted_chains;
  DeletedBufferChain *result = (DeletedBufferChain *)&*deleted_chains.insert(DeletedBufferChain(buffer_size)).first;
  lock.unlock();
  return result;
}
