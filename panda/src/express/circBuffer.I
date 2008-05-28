// Filename: circBuffer.I
// Created by:  drose (08Feb99)
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

#include "pandabase.h"
#include "config_express.h"
#include "pnotify.h"

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
//     Function: CircBuffer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE CircBuffer<Thing, max_size>::
~CircBuffer() {
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::size
//       Access: Public
//  Description: Returns the number of items currently in the buffer.
//               This can safely be called without synchronization
//               from either the reader or the writer thread, but the
//               size may of course vary without warning after the
//               call.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE int CircBuffer<Thing, max_size>::
size() const {
  int diff = _in - _out;
  return (diff >= 0) ? diff : max_size + 1 + diff;
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::empty
//       Access: Public
//  Description: Returns true if the buffer is empty.  It is safe to
//               call this without synchronization primitives from
//               either the reader or the writer thread, but the
//               result may vary without warning after the call.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE bool CircBuffer<Thing, max_size>::
empty() const {
  return _in == _out;
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::full
//       Access: Public
//  Description: Returns true if the buffer is full; if this is true,
//               push_back() will fail.  It is safe to call this
//               without synchronization primitives from either the
//               reader or the writer thread, but the result may vary
//               without warning after the call.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE bool CircBuffer<Thing, max_size>::
full() const {
  //  return _in == _out-1 || (_in==max_size && _out==0);
  return ((_in + 1) % (max_size + 1)) == _out;
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::front
//       Access: Public
//  Description: Returns a reference to the first item in the queue.
//               It is invalid to call this if empty() is true.  It is
//               safe to call this without synchronization only from
//               the reading thread: the thread that calls pop_front().
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE const Thing &CircBuffer<Thing, max_size>::
front() const {
  nassertr(!empty(), _array[0]);
  return _array[_out];
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::front
//       Access: Public
//  Description: Returns a reference to the first item in the queue.
//               It is invalid to call this if empty() is true.  It is
//               safe to call this without synchronization only from
//               the reading thread: the thread that calls pop_front().
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE Thing &CircBuffer<Thing, max_size>::
front() {
  nassertr(!empty(), _array[0]);
  return _array[_out];
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::operator []
//       Access: Public
//  Description: Returns the nth element in the buffer.  It is safe to
//               call this without synchronization only from the
//               reading thread: the thread that calls pop_front().
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE const Thing &CircBuffer<Thing, max_size>::
operator[] (int n) const {
  nassertr(!empty(), _array[0]);
  return _array[(_out + n) % (max_size + 1)];
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::operator []
//       Access: Public
//  Description: Returns the nth element in the buffer.  It is safe to
//               call this without synchronization only from the
//               reading thread: the thread that calls pop_front().
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE Thing &CircBuffer<Thing, max_size>::
operator[] (int n) {
  nassertr(!empty(), _array[0]);
  return _array[(_out + n) % (max_size + 1)];
}


////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::pop_front
//       Access: Public
//  Description: Removes the first item from the buffer.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE void CircBuffer<Thing, max_size>::
pop_front() {
  nassertv(!empty());

  // We need to clear out the old element to force its destructor to
  // be called; it might be important.  This will generate yet another
  // UMR warning in Purify if the default constructor doesn't fully
  // initialize the class.
  _array[_out] = Thing();

  _out = (_out+1)%(max_size+1);
}




////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::back
//       Access: Public
//  Description: Returns a reference to the last item in the queue.
//               It is invalid to call this if empty() is true.  It is
//               safe to call this without synchronization primitives
//               only from the writing thread: the thread that calls
//               push_back().
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE const Thing &CircBuffer<Thing, max_size>::
back() const {
  nassertr(!empty(), _array[0]);
  return _array[(_in + max_size) % (max_size + 1)];
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::back
//       Access: Public
//  Description: Returns a reference to the last item in the queue.
//               It is invalid to call this if empty() is true.  It is
//               safe to call this without synchronization primitives
//               only from the writing thread: the thread that calls
//               push_back().
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE Thing &CircBuffer<Thing, max_size>::
back() {
  nassertr(!empty(), _array[0]);
  return _array[(_in + max_size) % (max_size + 1)];
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::push_back
//       Access: Public
//  Description: Adds an item to the end of the buffer.  This may fail
//               if full() is true.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE void CircBuffer<Thing, max_size>::
push_back(const Thing &t) {
  if (full()) {
    express_cat.error()
      << "Circular buffer is full; cannot add requests.\n";
  } else {
    _array[_in] = t;
    _in = (_in+1)%(max_size+1);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CircBuffer::clear
//       Access: Public
//  Description: Removes all items from the queue.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
INLINE void CircBuffer<Thing, max_size>::
clear() {
  _in = _out = 0;
}
