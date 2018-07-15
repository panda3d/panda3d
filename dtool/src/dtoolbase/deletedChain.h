/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file deletedChain.h
 * @author drose
 * @date 2006-04-01
 */

#ifndef DELETEDCHAIN_H
#define DELETEDCHAIN_H

#include "dtoolbase.h"
#include "deletedBufferChain.h"
#include <assert.h>

/**
 * This template class can be used to provide faster allocation/deallocation
 * for many Panda objects.  It works by maintaining a linked list of deleted
 * objects that are all of the same type; when a new object is allocated that
 * matches that type, the same space is just reused.
 *
 * This class is actually a layer on top of DeletedBufferChain, which handles
 * the actual allocation.  This class just provides the typecasting.
 *
 * Of course, this trick of maintaining the deleted object chain won't work in
 * the presence of polymorphism, where you might have many classes that derive
 * from a base class, and all of them have a different size--unless you
 * instantiate a DeletedChain for *every* kind of derived class.  The
 * ALLOC_DELETED_CHAIN macro, below, is designed to make this easy.
 */
template<class Type>
class DeletedChain {
public:
  INLINE Type *allocate(size_t size, TypeHandle type_handle);
  INLINE void deallocate(Type *ptr, TypeHandle type_handle);

  INLINE bool validate(const Type *ptr);

  static INLINE ReferenceCount *make_ref_ptr(void *ptr);
  static INLINE ReferenceCount *make_ref_ptr(ReferenceCount *ptr);

private:
  INLINE void init_deleted_chain();

  DeletedBufferChain *_chain;
};

/**
 * This template class is used to conveniently declare a single instance of
 * the DeletedChain template object, above, for a particular type.
 *
 * It relies on the fact that the compiler and linker should unify all
 * references to this static pointer for a given type, as per the C++ spec.
 * However, this sometimes fails; and if the compiler fails to do this, it
 * mostly won't be a big deal; it just means there will be multiple unrelated
 * chains of deleted objects for a particular type.  This is only a problem if
 * the code structure causes objects to be allocated from one chain and freed
 * to another, which can lead to leaks.
 */
template<class Type>
class StaticDeletedChain {
public:
  INLINE static Type *allocate(size_t size, TypeHandle type_handle);
  INLINE static void deallocate(Type *ptr, TypeHandle type_handle);

  INLINE static bool validate(const Type *ptr);

  static DeletedChain<Type> _chain;
};

#ifdef USE_DELETED_CHAIN
// Place this macro within a class definition to define appropriate operator
// new and delete methods that take advantage of DeletedChain.
#define ALLOC_DELETED_CHAIN(Type)                            \
  inline void *operator new(size_t size) RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT) { \
    return (void *)StaticDeletedChain< Type >::allocate(size, get_type_handle(Type)); \
  }                                                          \
  inline void *operator new(size_t size, void *ptr) {        \
    (void) size;                                             \
    return ptr;                                              \
  }                                                          \
  inline void operator delete(void *ptr) {                   \
    StaticDeletedChain< Type >::deallocate((Type *)ptr, get_type_handle(Type)); \
  }                                                          \
  inline void operator delete(void *, void *) {              \
  }                                                          \
  inline static bool validate_ptr(const void *ptr) {         \
    return StaticDeletedChain< Type >::validate((const Type *)ptr); \
  }

// Use this variant of the above macro in cases in which the compiler fails to
// unify the static template pointers properly, to prevent leaks.
#define ALLOC_DELETED_CHAIN_DECL(Type)                       \
  inline void *operator new(size_t size) RETURNS_ALIGNED(MEMORY_HOOK_ALIGNMENT) { \
    return (void *)_deleted_chain.allocate(size, get_type_handle(Type)); \
  }                                                          \
  inline void *operator new(size_t size, void *ptr) {        \
    (void) size;                                             \
    return ptr;                                              \
  }                                                          \
  inline void operator delete(void *ptr) {                   \
    _deleted_chain.deallocate((Type *)ptr, get_type_handle(Type)); \
  }                                                          \
  inline void operator delete(void *, void *) {              \
  }                                                          \
  inline static bool validate_ptr(const void *ptr) {         \
    return _deleted_chain.validate((const Type *)ptr);       \
  }                                                          \
  static DeletedChain< Type > _deleted_chain;

// When you use ALLOC_DELETED_CHAIN_DECL in a class body, you must also put
// this line in the .cxx file defining that class body.
#define ALLOC_DELETED_CHAIN_DEF(Type)                        \
  DeletedChain< Type > Type::_deleted_chain;

#else  // USE_DELETED_CHAIN

#define ALLOC_DELETED_CHAIN(Type)                            \
  inline static bool validate_ptr(const void *ptr) {         \
    return (ptr != nullptr);                                    \
  }
#define ALLOC_DELETED_CHAIN_DECL(Type)                       \
  inline static bool validate_ptr(const void *ptr) {         \
    return (ptr != nullptr);                                    \
  }
#define ALLOC_DELETED_CHAIN_DEF(Type)

#endif  // USE_DELETED_CHAIN

#include "deletedChain.T"

#endif
