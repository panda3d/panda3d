// Filename: plist.h
// Created by:  drose (05Jun01)
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

#ifndef PLIST_H
#define PLIST_H

#include "dtoolbase.h"
#include "pallocator.h"
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
  typedef TYPENAME list<Type, pallocator_single<Type> >::size_type size_type;
  plist() : list<Type, pallocator_single<Type> >() { }
  plist(const plist<Type> &copy) : list<Type, pallocator_single<Type> >(copy) { }
  plist(size_type n) : list<Type, pallocator_single<Type> >(n) { }
  plist(size_type n, const Type &value) : list<Type, pallocator_single<Type> >(n, value) { }

  typedef TYPENAME list<Type, pallocator_single<Type> >::iterator iterator;
  typedef TYPENAME list<Type, pallocator_single<Type> >::const_iterator const_iterator;
  typedef TYPENAME list<Type, pallocator_single<Type> >::reverse_iterator reverse_iterator;
  typedef TYPENAME list<Type, pallocator_single<Type> >::const_reverse_iterator const_reverse_iterator;
};

#endif  // USE_STL_ALLOCATOR
#endif
