// Filename: uniqueIdAllocator.cxx
// Created by:  schuyler 2003-03-13
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
// 

#include "pandabase.h"
#include "notify.h"

#include "uniqueIdAllocator.h"

NotifyCategoryDecl(uniqueIdAllocator, EXPCL_PANDA, EXPTP_PANDA);
NotifyCategoryDef(uniqueIdAllocator, "");

const U32 UniqueIdAllocator::IndexEnd=(U32)-1;
const U32 UniqueIdAllocator::IndexAllocated=(U32)-2;

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
  _table[_size-1]=IndexEnd;
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
//               IndexEnd is returned if no ids are available.
////////////////////////////////////////////////////////////////////
U32 UniqueIdAllocator::
allocate() {
  if (_next_free==IndexEnd) {
    // ...all ids allocated.
    uniqueIdAllocator_warning("allocate Error: no more free ids.");
    return IndexEnd;
  }
  U32 id=_min+_next_free;
  _next_free=_table[_next_free];
  #ifndef NDEBUG //[
    _table[id-_min]=IndexAllocated;
  #endif //]
  --_free;
  uniqueIdAllocator_debug("allocate() returning "<<id);
  return id;
}


////////////////////////////////////////////////////////////////////
//     Function: free
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
  nassertv(_table[index]==IndexAllocated); // Attempt to free non-allocated id.
  _table[index]=IndexEnd; // Mark this element as the end of the list.
  _table[_last_free]=index;
  if (_next_free==IndexEnd) {
    // ...the free list was empty.
    _next_free=index;
  }
  _last_free=index;
  ++_free;
}


////////////////////////////////////////////////////////////////////
//     Function: fraction_used
//       Access: 
//  Description: return the decimal fraction of the pool that is used.
//               The range is 0 to 1.0 (e.g. 75% would be 0.75).
////////////////////////////////////////////////////////////////////
float UniqueIdAllocator::
fraction_used() const {
  return float(_size-_free)/_size;
}


////////////////////////////////////////////////////////////////////
//     Function: output
//       Access: 
//  Description: ...intended for debugging only.
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
output(ostream& os, bool verbose) const {
  os  <<"[_min: "<<_min<<"; _max: "<<_max
      <<";\n_next_free: "<<long(_next_free)
      <<"; _last_free: "<<long(_last_free)
      <<"; _size: "<<_size
      <<"; _free: "<<_free
      <<"; used: "<<_size-_free
      <<"; fraction_used: "<<fraction_used();
  if (verbose) {
    os <<";\n     ";
    for (U32 i=0; i<_size; ++i) {
      os<<long(_table[i])<<", ";
    }
  }
  os<<"]"<<endl;
}

