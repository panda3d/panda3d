/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file typeHandle.cxx
 * @author drose
 * @date 1998-10-23
 */

#include "typeHandle.h"
#include "typeRegistryNode.h"

/**
 * Returns the total allocated memory used by objects of this type, for the
 * indicated memory class.  This is only updated if track-memory-usage is set
 * true in your Config.prc file.
 */
size_t TypeHandle::
get_memory_usage(MemoryClass memory_class) const {
#ifdef DO_MEMORY_USAGE
  assert((int)memory_class >= 0 && (int)memory_class < (int)MC_limit);
  if ((*this) == TypeHandle::none()) {
    return 0;
  } else {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
    assert(rnode != nullptr);
    return rnode->_memory_usage[memory_class].load(std::memory_order_relaxed);
  }
#endif  // DO_MEMORY_USAGE
  return 0;
}

/**
 * Adds the indicated amount to the record for the total allocated memory for
 * objects of this type.
 */
void TypeHandle::
inc_memory_usage(MemoryClass memory_class, size_t size) {
#ifdef DO_MEMORY_USAGE
#ifdef _DEBUG
  assert((int)memory_class >= 0 && (int)memory_class < (int)MC_limit);
#endif
  if ((*this) != TypeHandle::none()) {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
    assert(rnode != nullptr);
    size_t prev = rnode->_memory_usage[memory_class].fetch_add(size, std::memory_order_relaxed);
    if (prev + size < prev) {
      std::cerr << "Memory usage overflow for type " << rnode->_name << ".\n";
      abort();
    }
  }
#endif  // DO_MEMORY_USAGE
}

/**
 * Subtracts the indicated amount from the record for the total allocated
 * memory for objects of this type.
 */
void TypeHandle::
dec_memory_usage(MemoryClass memory_class, size_t size) {
#ifdef DO_MEMORY_USAGE
#ifdef _DEBUG
  assert((int)memory_class >= 0 && (int)memory_class < (int)MC_limit);
#endif
  if ((*this) != TypeHandle::none()) {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
    assert(rnode != nullptr);
    size_t prev = rnode->_memory_usage[memory_class].fetch_sub(size, std::memory_order_relaxed);
    assert(prev - size <= prev);
  }
#endif  // DO_MEMORY_USAGE
}

/**
 * Allocates memory, adding it to the total amount of memory allocated for
 * this type.
 */
void *TypeHandle::
allocate_array(size_t size) {
  TAU_PROFILE("TypeHandle:allocate_array()", " ", TAU_USER);

  void *ptr = PANDA_MALLOC_ARRAY(size);
#ifdef DO_MEMORY_USAGE
  if ((*this) != TypeHandle::none()) {
    size_t alloc_size = MemoryHook::get_ptr_size(ptr);
#ifdef _DEBUG
    assert(size <= alloc_size);
#endif
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
    assert(rnode != nullptr);
    size_t prev = rnode->_memory_usage[MC_array].fetch_add(alloc_size, std::memory_order_relaxed);
    if (prev + size < prev) {
      std::cerr << "Memory usage overflow for type " << rnode->_name << ".\n";
      abort();
    }
  }
#endif  // DO_MEMORY_USAGE
  return ptr;
}

/**
 * Reallocates memory, adjusting the total amount of memory allocated for this
 * type.
 */
void *TypeHandle::
reallocate_array(void *old_ptr, size_t size) {
  TAU_PROFILE("TypeHandle:reallocate_array()", " ", TAU_USER);

#ifdef DO_MEMORY_USAGE
  size_t old_size = MemoryHook::get_ptr_size(old_ptr);
  void *new_ptr = PANDA_REALLOC_ARRAY(old_ptr, size);

  if ((*this) != TypeHandle::none()) {
    size_t new_size = MemoryHook::get_ptr_size(new_ptr);

    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
    assert(rnode != nullptr);
    if (new_size > old_size) {
      rnode->_memory_usage[MC_array].fetch_add(new_size - old_size, std::memory_order_relaxed);
    } else {
      rnode->_memory_usage[MC_array].fetch_sub(old_size - new_size, std::memory_order_relaxed);
    }
  }
#else
  void *new_ptr = PANDA_REALLOC_ARRAY(old_ptr, size);
#endif
  return new_ptr;
}

/**
 * Deallocates memory, subtracting it from the total amount of memory
 * allocated for this type.
 */
void TypeHandle::
deallocate_array(void *ptr) {
  TAU_PROFILE("TypeHandle:deallocate_array()", " ", TAU_USER);

#ifdef DO_MEMORY_USAGE
  size_t alloc_size = MemoryHook::get_ptr_size(ptr);
  if ((*this) != TypeHandle::none()) {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
    assert(rnode != nullptr);
    size_t prev = rnode->_memory_usage[MC_array].fetch_sub(alloc_size, std::memory_order_relaxed);
    assert(prev - alloc_size <= prev);
  }
#endif  // DO_MEMORY_USAGE
  PANDA_FREE_ARRAY(ptr);
}

#ifdef HAVE_PYTHON
/**
 * Returns the internal void pointer that is stored for interrogate's benefit.
 */
PyTypeObject *TypeHandle::
get_python_type() const {
  TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
  if (rnode != nullptr) {
    return rnode->get_python_type();
  } else {
    return nullptr;
  }
}

/**
 * Returns a Python wrapper object corresponding to the given C++ pointer.
 */
PyObject *TypeHandle::
wrap_python(void *ptr, PyTypeObject *cast_from) const {
  if (ptr == nullptr) {
    return nullptr;
  }
  TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, nullptr);
  if (rnode != nullptr) {
    return rnode->wrap_python(ptr, cast_from);
  } else {
    return nullptr;
  }
}
#endif

std::ostream &
operator << (std::ostream &out, TypeHandle::MemoryClass mem_class) {
  switch (mem_class) {
  case TypeHandle::MC_singleton:
    return out << "singleton";

  case TypeHandle::MC_array:
    return out << "array";

  case TypeHandle::MC_deleted_chain_active:
    return out << "deleted_chain_active";

  case TypeHandle::MC_deleted_chain_inactive:
    return out << "deleted_chain_inactive";

  case TypeHandle::MC_limit:
    return out << "limit";
  }

  return out
    << "**invalid TypeHandle::MemoryClass (" << (int)mem_class
    << ")**\n";
}
