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

////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: Create a free id pool in the range [min:max].
////////////////////////////////////////////////////////////////////
UniqueIdAllocator::
UniqueIdAllocator(U32 min, U32 max)
  : _min(min), _max(max) {
  //cout<<"UniqueIdAllocator::UniqueIdAllocator("<<min<<", "<<max<<")"<<endl;
  _size=_max-_min+1; // +1 because min and max are inclusive.
  assert(_size); // size must be > 0.
  _table=new U32[_size];
  assert(_table); // This should be redundant if new throws an exception.
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
  //cout<<"UniqueIdAllocator::~UniqueIdAllocator()"<<endl;
  delete [] _table;
}


////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: Receive an id between _min and _max (that were passed
//               to the constructor).  This code will succede or call
//               exit().
////////////////////////////////////////////////////////////////////
U32 UniqueIdAllocator::
allocate() {
  if (_next_free==-1) {
    // ...all ids allocated.
    cerr<<"UniqueIdAllocator Error: all ids allocated."<<endl;
    // TODO:throw an exception rather than calling exit.
    exit(1);
  }
  // This next block is redundant with the one above it, but I'm leaving
  // the one above in place, in case anyone removes this next block.
  if (_free<=(_size>>2)) {
    // ...under 1/4 of the ids are free.
    cerr<<"UniqueIdAllocator Error: 75% of ids allocated."<<endl;
    // TODO:throw an exception rather than calling exit.
    exit(1);
  }
  U32 id=_min+_next_free;
  _next_free=_table[_next_free];
  assert(_table[id-_min]=-2); // this assignment is debug only.
  --_free;
  //cout<<"UniqueIdAllocator::allocate() returning "<<id<<endl;
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
  //cout<<"UniqueIdAllocator::free(index)"<<endl;
  assert(index>=_min); // Attempt to free out-of-range id.
  assert(index<=_max); // Attempt to free out-of-range id.
  index=index-_min;
  assert(_table[index]==-2); // Attempt to free non-allocated id.
  _table[index]=-1;
  _table[_last_free]=index;
  #if 0 //[
  // This is only necessary if the free pool is allowed to go empty.
  // Since we don't allow that, it is an optimization to comment
  // this out.
  if (_next_free==-1) {
    _next_free=index;
  }
  #endif //]
  _last_free=index;
  ++_free;
}


////////////////////////////////////////////////////////////////////
//     Function: 
//       Access: 
//  Description: ...intended for debugging only.
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
printTo(ostream& os) const {
  os  <<"[_next_free: "<<long(_next_free)
      <<"; _last_free: "<<long(_last_free)
      <<"; _size: "<<_size
      <<"; _free: "<<_free
      <<"; used: "<<_size-_free
      <<"; %used: "<<float(_size-_free)/_size // This differs the %used code above.
      <<";\n     ";
  for (U32 i=0; i<_size; ++i) {
    os<<long(_table[i])<<", ";
  }
  os<<"]"<<endl;
}

