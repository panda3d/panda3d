// Filename: plist.h
// Created by:  drose (05Jun01)
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

#ifndef PLIST_H
#define PLIST_H

#include "dtoolbase.h"
#include "pallocator.h"
#include "register_type.h"
#include <list>

#ifndef USE_STL_ALLOCATOR
// If we're not using custom allocators, just use the standard class
// definition.
#define plist list

#else

////////////////////////////////////////////////////////////////////
//       Class : plist
// Description : This is our own Panda specialization on the default
//               STL list.  Its main purpose is to call the hooks
//               for MemoryUsage to properly track STL-allocated
//               memory.
////////////////////////////////////////////////////////////////////
template<class Type>
class plist : public list<Type, pallocator_single<Type> > {
public:
  typedef pallocator_single<Type> allocator;
  typedef list<Type, allocator> base_class;
  typedef TYPENAME base_class::size_type size_type;
  plist(TypeHandle type_handle = plist_type_handle) : base_class(allocator(type_handle)) { }
  plist(const plist<Type> &copy) : base_class(copy) { }
  plist(size_type n, TypeHandle type_handle = plist_type_handle) : base_class(n, Type(), allocator(type_handle)) { }
  plist(size_type n, const Type &value, TypeHandle type_handle = plist_type_handle) : base_class(n, value, allocator(type_handle)) { }

  typedef TYPENAME base_class::iterator iterator;
  typedef TYPENAME base_class::const_iterator const_iterator;
  typedef TYPENAME base_class::reverse_iterator reverse_iterator;
  typedef TYPENAME base_class::const_reverse_iterator const_reverse_iterator;

  // This exists because libc++'s remove implementation has a bug with
  // Panda's allocator class.
  INLINE void remove(const Type &val) {
    iterator it = this->begin();
    while (it != this->end()) {
      if (*it == val) {
        it = this->erase(it);
      } else {
        ++it;
      }
    }
  };
};

#endif  // USE_STL_ALLOCATOR
#endif
