// Filename: circBuffer.h
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

#ifndef CIRCBUFFER_H
#define CIRCBUFFER_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : CircBuffer
// Description : This class implements a queue of some type via a
//               circular buffer.  The circular buffer has the
//               advantage that no synchronization is required when
//               one process adds to the queue while another process
//               extracts.  It works for any kind of Thing that has a
//               valid assignment operator and copy constructor
//               defined.
////////////////////////////////////////////////////////////////////
template<class Thing, int max_size>
class CircBuffer {
public:
  INLINE CircBuffer();
  INLINE ~CircBuffer();

  // Methods that are safe to call without synchronization primitives
  // from either thread.
  INLINE int size() const;

  // Methods that are safe to call without synchronization primitives
  // only from the reader thread.
  INLINE bool empty() const;

  INLINE const Thing &front() const;
  INLINE Thing &front();
  INLINE void pop_front();

  INLINE const Thing &operator[] (int n) const;
  INLINE Thing &operator[] (int n);

  // Methods that are safe to call without synchronization primitives
  // only from the writer thread.
  INLINE bool full() const;

  INLINE const Thing &back() const;
  INLINE Thing &back();
  INLINE void push_back(const Thing &t);

private:
  Thing _array[max_size+1];
  int _in, _out;
};

#include "circBuffer.I"

#endif
