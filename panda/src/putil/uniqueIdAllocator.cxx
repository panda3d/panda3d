// Filename: uniqueIdAllocator.cxx
// Created by:  schuyler 2003-03-13
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////
// 

#include "pandabase.h"
#include "notify.h"

#include "uniqueIdAllocator.h"

NotifyCategoryDecl(uniqueIdAllocator, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDef(uniqueIdAllocator, "");

#ifndef NDEBUG //[
  // Non-release build:
  #define uniqueIdAllocator_debug(msg) \
  if (uniqueIdAllocator_cat.is_debug()) { \
    uniqueIdAllocator_cat->debug() << msg << endl; \
  } else {}

  #define uniqueIdAllocator_info(msg) \
    uniqueIdAllocator_cat->info() << msg << endl

  #define uniqueIdAllocator_warning(msg) \
    uniqueIdAllocator_cat->warning() << msg << endl
#else //][
  // Release build:
  #define uniqueIdAllocator_debug(msg) ((void)0)
  #define uniqueIdAllocator_info(msg) ((void)0)
  #define uniqueIdAllocator_warning(msg) ((void)0)
#endif //]

#define audio_error(msg) \
  audio_cat->error() << msg << endl

////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: Create a free id pool in the range [min:max].
////////////////////////////////////////////////////////////////////
UniqueIdAllocator::
UniqueIdAllocator(U32 min, U32 max)
  : _min(min), _max(max) {
  uniqueIdAllocator_debug("UniqueIdAllocator("<<min<<", "<<max<<")");
  _size=_max-_min+1; // +1 because min and max are inclusive.
  nassertv(_size); // size must be > 0.
  _table=new U32[_size];
  nassertv(_table); // This should be redundant if new throws an exception.
  for (U32 i=0; i<_size; ++i) {
    _table[i]=i+1;
  }
  _table[_size-1]=-1;
  _next_free=0;
  _last_free=_size-1;
  _free=_size;
}

////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: 
////////////////////////////////////////////////////////////////////
UniqueIdAllocator::
~UniqueIdAllocator() {
  uniqueIdAllocator_debug("~UniqueIdAllocator()");
  delete [] _table;
}


////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: Receive an id between _min and _max (that were passed
//               to the constructor).
//               -1 is returned if no ids are available.
////////////////////////////////////////////////////////////////////
U32 UniqueIdAllocator::
allocate() {
  if (_next_free==-1) {
    // ...all ids allocated.
    uniqueIdAllocator_warning("allocate Error: no more free ids.");
    return -1;
  }
  U32 id=_min+_next_free;
  _next_free=_table[_next_free];
  nassertr(_table[id-_min]=-2, -1); // this assignment is debug only.
  --_free;
  uniqueIdAllocator_debug("allocate() returning "<<id);
  return id;
}


////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: Free an allocated index (index must be between _min
//               and _max that were passed to the constructor).
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
free(U32 index) {
  uniqueIdAllocator_debug("free("<<index<<")");
  nassertv(index>=_min); // Attempt to free out-of-range id.
  nassertv(index<=_max); // Attempt to free out-of-range id.
  index=index-_min; // Convert to _table index.
  nassertv(_table[index]==-2); // Attempt to free non-allocated id.
  _table[index]=-1; // Mark this element as the end of the list.
  _table[_last_free]=index;
  if (_next_free==-1) {
    // ...the free list was empty.
    _next_free=index;
  }
  _last_free=index;
  ++_free;
}


////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: return what percentage of the pool is used.  The 
//               range is 0 to 1.0, so 75% would be 0.75, for example.
////////////////////////////////////////////////////////////////////
float UniqueIdAllocator::
percent_used() const {
  return float(_size-_free)/_size;
}


////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: ...intended for debugging only.
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
output(ostream& os, bool verbose) const {
  os  <<"[_next_free: "<<long(_next_free)
      <<"; _last_free: "<<long(_last_free)
      <<"; _size: "<<_size
      <<"; _free: "<<_free
      <<"; used: "<<_size-_free
      <<"; %used: "<<float(_size-_free)/_size;
  if (verbose) {
    os <<";\n     ";
    for (U32 i=0; i<_size; ++i) {
      os<<long(_table[i])<<", ";
    }
  }
  os<<"]"<<endl;
}

