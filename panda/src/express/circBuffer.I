// Filename: circBuffer.I
// Created by:  drose (08Feb99)
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

#include <pandabase.h>

#include "config_express.h"

#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE CircBuffer<Thing, max_size>::
CircBuffer() {
  _in = _out = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::is_empty
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE bool CircBuffer<Thing, max_size>::
is_empty() const {
  return _in == _out;
}


////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::is_full
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE bool CircBuffer<Thing, max_size>::
is_full() const {
  return _in == _out-1 || (_in==max_size && _out==0);
}


////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::peek
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE const Thing &CircBuffer<Thing, max_size>::
peek() const {
  nassertr(!is_empty(), *(new Thing));
  return _array[_out];
}


////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::extract
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE Thing CircBuffer<Thing, max_size>::
extract() {
  nassertr(!is_empty(), Thing());
  Thing temp = _array[_out];

  // We need to clear out the old element to force its destructor to
  // be called; it might be important.  This will generate yet another
  // UMR warning in Purify if the default constructor doesn't fully
  // initialize the class.
  _array[_out] = Thing();

  _out = (_out+1)%(max_size+1);
  return temp;
}


////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::insert
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE void CircBuffer<Thing, max_size>::
insert(const Thing &t) {
  if (is_full()) {
    express_cat.error()
      << "Circular buffer is full; cannot add requests.\n";
  } else {
    _array[_in] = t;
    _in = (_in+1)%(max_size+1);
  }
}
