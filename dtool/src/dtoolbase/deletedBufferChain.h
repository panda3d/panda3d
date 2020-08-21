/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file deletedBufferChain.h
 * @author drose
 * @date 2007-07-20
 */

#ifndef DELETEDBUFFERCHAIN_H
#define DELETEDBUFFERCHAIN_H

#include "dtoolbase.h"
#include "neverFreeMemory.h"
#include "mutexImpl.h"
#include "atomicAdjust.h"
#include "numeric_types.h"
#include "typeHandle.h"
#include <assert.h>

// Though it's tempting, it doesn't seem to be possible to implement
// DeletedBufferChain via the atomic exchange operation.  Specifically, a
// pointer may be removed from the head of the chain, then the same pointer
// reinserted in the chain, while another thread is waiting; and that thread
// will not detect the change.  So instead, we always use a mutex.

#ifndef NDEBUG
// In development mode, we define USE_DELETEDCHAINFLAG, which triggers the
// piggyback of an additional word of data on every allocated block, so we can
// ensure that an object is not double-deleted and that the deleted chain
// remains intact.
#define USE_DELETEDCHAINFLAG 1
#endif // NDEBUG

#ifdef USE_DELETEDCHAINFLAG
enum DeletedChainFlag {
  DCF_deleted = 0xfeedba0f,
  DCF_alive = 0x12487654,
};
#endif

/**
 * This template class can be used to provide faster allocation/deallocation
 * for many Panda objects.  It works by maintaining a linked list of deleted
 * buffers that are all of the same size; when a new object is allocated that
 * matches that size, the same space is just reused.
 *
 * This class manages untyped buffers of a fixed size.  It can be used
 * directly; or it also serves as a backbone for DeletedChain, which is a
 * template class that manages object allocations.
 *
 * Use MemoryHook to get a new DeletedBufferChain of a particular size.
 */
class EXPCL_DTOOL_DTOOLBASE DeletedBufferChain {
protected:
  DeletedBufferChain(size_t buffer_size);

public:
  void *allocate(size_t size, TypeHandle type_handle);
  void deallocate(void *ptr, TypeHandle type_handle);

  INLINE bool validate(void *ptr);
  INLINE size_t get_buffer_size() const;

private:
  class ObjectNode {
  public:
#ifdef USE_DELETEDCHAINFLAG
    // In development mode, we piggyback this extra data.  This is maintained
    // out-of-band from the actual pointer returned, so we can safely use this
    // flag to indicate the difference between allocated and freed pointers.
    TVOLATILE AtomicAdjust::Integer _flag;
#endif

    // This pointer sits within the buffer, in the same space referenced by
    // the actual pointer returned (unlike _flag, above).  It's only used when
    // the buffer is deleted, so there's no harm in sharing space with the
    // undeleted buffer.
    ObjectNode *_next;
  };

  static INLINE void *node_to_buffer(ObjectNode *node);
  static INLINE ObjectNode *buffer_to_node(void *buffer);

  ObjectNode *_deleted_chain;

  MutexImpl _lock;
  size_t _buffer_size;

#ifndef USE_DELETEDCHAINFLAG
  // Without DELETEDCHAINFLAG, we don't even store the _flag member at all.
  static const size_t flag_reserved_bytes = 0;

#else
  // Otherwise, we need space for the integer.
  static const size_t flag_reserved_bytes = sizeof(AtomicAdjust::Integer);
#endif  // USE_DELETEDCHAINFLAG

  friend class MemoryHook;
};

#include "deletedBufferChain.I"

#endif
