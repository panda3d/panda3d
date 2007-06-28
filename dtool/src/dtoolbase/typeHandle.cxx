// Filename: typeHandle.cxx
// Created by:  drose (23Oct98)
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

#include "typeHandle.h"
#include "typeRegistryNode.h"
#include "atomicAdjust.h"

// This is initialized to zero by static initialization.
TypeHandle TypeHandle::_none;

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::get_memory_usage
//       Access: Published
//  Description: Returns the total allocated memory used by objects of
//               this type, for the indicated memory class.  This is
//               only updated if track-memory-usage is set true in
//               your Config.prc file.
////////////////////////////////////////////////////////////////////
int TypeHandle::
get_memory_usage(MemoryClass memory_class) const {
  assert((int)memory_class >= 0 && (int)memory_class < (int)MC_limit);
  if ((*this) == TypeHandle::none()) {
    return 0;
  } else {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, NULL);
    assert(rnode != (TypeRegistryNode *)NULL);
    return (size_t)AtomicAdjust::get(rnode->_memory_usage[memory_class]);
  }
}
#endif  // DO_MEMORY_USAGE

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::inc_memory_usage
//       Access: Published
//  Description: Adds the indicated amount to the record for the total
//               allocated memory for objects of this type.
////////////////////////////////////////////////////////////////////
void TypeHandle::
inc_memory_usage(MemoryClass memory_class, int size) {
  assert((int)memory_class >= 0 && (int)memory_class < (int)MC_limit);
  if ((*this) != TypeHandle::none()) {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, NULL);
    assert(rnode != (TypeRegistryNode *)NULL);
    AtomicAdjust::add(rnode->_memory_usage[memory_class], (PN_int32)size);
    assert(rnode->_memory_usage[memory_class] >= 0);
  }
}
#endif  // DO_MEMORY_USAGE

#ifdef DO_MEMORY_USAGE
////////////////////////////////////////////////////////////////////
//     Function: TypeHandle::dec_memory_usage
//       Access: Published
//  Description: Subtracts the indicated amount from the record for
//               the total allocated memory for objects of this type.
////////////////////////////////////////////////////////////////////
void TypeHandle::
dec_memory_usage(MemoryClass memory_class, int size) {
  assert((int)memory_class >= 0 && (int)memory_class < (int)MC_limit);
  if ((*this) != TypeHandle::none()) {
    TypeRegistryNode *rnode = TypeRegistry::ptr()->look_up(*this, NULL);
    assert(rnode != (TypeRegistryNode *)NULL);
    AtomicAdjust::add(rnode->_memory_usage[memory_class], -(PN_int32)size);
    assert(rnode->_memory_usage[memory_class] >= 0);
  }
}
#endif  // DO_MEMORY_USAGE

ostream &
operator << (ostream &out, TypeHandle::MemoryClass mem_class) {
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
