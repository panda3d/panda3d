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

#include <pandabase.h>

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

  INLINE bool is_empty() const;
  INLINE bool is_full() const;

  INLINE const Thing &peek() const;
  INLINE Thing extract();

  INLINE void insert(const Thing &t);

protected:
  Thing _array[max_size+1];
  int _in, _out;
};

#include "circBuffer.I"

#endif
