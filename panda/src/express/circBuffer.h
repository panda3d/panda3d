// Filename: circBuffer.h
// Created by:  drose (08Feb99)
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
