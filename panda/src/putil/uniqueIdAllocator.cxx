// Filename: uniqueIdAllocator.cxx
// Created by:  schuyler 2003-03-13
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
// 

#include "pandabase.h"
#include "pnotify.h"
#include "notifyCategoryProxy.h"

#include "uniqueIdAllocator.h"

NotifyCategoryDecl(uniqueIdAllocator, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);
NotifyCategoryDef(uniqueIdAllocator, "");

const PN_uint32 UniqueIdAllocator::IndexEnd = (PN_uint32)-1;
const PN_uint32 UniqueIdAllocator::IndexAllocated = (PN_uint32)-2;

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
//     Function: UniqueIdAllocator::Constructor
//       Access: Published
//  Description: Create a free id pool in the range [min:max].
////////////////////////////////////////////////////////////////////
UniqueIdAllocator::
UniqueIdAllocator(PN_uint32 min, PN_uint32 max)
  : _min(min), _max(max) {
  uniqueIdAllocator_debug("UniqueIdAllocator("<<min<<", "<<max<<")");

  nassertv(_max >= _min);
  _size = _max-_min+1; // +1 because min and max are inclusive.
  nassertv(_size != 0); // size must be > 0.

  _table = (PN_uint32 *)PANDA_MALLOC_ARRAY(_size * sizeof(PN_uint32));
  nassertv(_table); // This should be redundant if new throws an exception.

  for (PN_uint32 i = 0; i < _size; ++i) {
    _table[i] = i + 1;
  }
  _table[_size - 1] = IndexEnd;
  _next_free = 0;
  _last_free = _size - 1;
  _free = _size;
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::Destructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
UniqueIdAllocator::
~UniqueIdAllocator() {
  uniqueIdAllocator_debug("~UniqueIdAllocator()");
  PANDA_FREE_ARRAY(_table);
}


////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::allocate
//       Access: Published
//  Description: Returns an id between _min and _max (that were passed
//               to the constructor).
//               IndexEnd is returned if no ids are available.
////////////////////////////////////////////////////////////////////
PN_uint32 UniqueIdAllocator::
allocate() {
  if (_next_free == IndexEnd) {
    // ...all ids allocated.
    uniqueIdAllocator_warning("allocate Error: no more free ids.");
    return IndexEnd;
  }
  PN_uint32 index = _next_free;
  nassertr(_table[index] != IndexAllocated, IndexEnd);

  _next_free = _table[_next_free];
  _table[index] = IndexAllocated;

  --_free;

  PN_uint32 id = index + _min;
  uniqueIdAllocator_debug("allocate() returning " << id);
  return id;
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::initial_reserve_id
//       Access: Published
//  Description: This may be called to mark a particular id as having
//               already been allocated (for instance, by a prior
//               pass).  The specified id is removed from the
//               available pool.
//
//               Because of the limitations of this algorithm, this is
//               most efficient when it is called before the first
//               call to allocate(), and when all the calls to
//               initial_reserve_id() are made in descending order by
//               id.  However, this is a performance warning only; if
//               performance is not an issue, any id may be reserved
//               at any time.
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
initial_reserve_id(PN_uint32 id) {
  nassertv(id >= _min && id <= _max); // Attempt to reserve out-of-range id.
  PN_uint32 index = id - _min; // Convert to _table index.

  nassertv(_table[index] != IndexAllocated);

  if (_free == 1) {
    // We just reserved the last element in the free chain.
    _next_free = IndexEnd;

  } else if (_next_free == index) {
    // We're reserving the head of the free chain.
    _next_free = _table[index];

  } else {
    // Since we don't store back pointers in the free chain, we have
    // to search for the element in the free chain that points to this
    // index.

    // However, there is an easy optimal case: because we expect that
    // this call will be made before any calls to allocate(),
    // hopefully is it still true that the _table is still set up such
    // that _table[i] = i+1 (and if the numbers are reserved in
    // descending order, this will be true at least for all i <=
    // index).  Thus, the free link to slot [index] is expected to be
    // the slot right before it, or if not, it usually won't be far
    // before it.

    PN_uint32 prev_index = index;
    while (prev_index > 0 && _table[prev_index - 1] != index) {
      --prev_index;
    }
    if (prev_index > 0 && _table[prev_index - 1] == index) {
      // We've found it.
      --prev_index;

    } else {
      // OK, it wasn't found below; we have to search above.
      prev_index = index + 1;
      while (prev_index < _size && _table[prev_index] != index) {
        ++prev_index;
      }
    }

    nassertv(_table[prev_index] == index);
    _table[prev_index] = _table[index];

    if (index == _last_free) {
      _last_free = prev_index;
    }
  }

  _table[index] = IndexAllocated;
  --_free;
}


////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::free
//       Access: Published
//  Description: Free an allocated index (index must be between _min
//               and _max that were passed to the constructor).
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
free(PN_uint32 id) {
  uniqueIdAllocator_debug("free("<<id<<")");

  nassertv(id >= _min && id <= _max); // Attempt to free out-of-range id.
  PN_uint32 index = id - _min; // Convert to _table index.
  nassertv(_table[index] == IndexAllocated); // Attempt to free non-allocated id.
  if (_next_free != IndexEnd) {
    nassertv(_table[_last_free] == IndexEnd);
    _table[_last_free] = index;
  }
  _table[index] = IndexEnd; // Mark this element as the end of the list.
  _last_free = index;

  if (_next_free == IndexEnd) {
    // ...the free list was empty.
    _next_free = index;
  }

  ++_free;
}


////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::fraction_used
//       Access: Published
//  Description: return the decimal fraction of the pool that is used.
//               The range is 0 to 1.0 (e.g. 75% would be 0.75).
////////////////////////////////////////////////////////////////////
PN_stdfloat UniqueIdAllocator::
fraction_used() const {
  return PN_stdfloat(_size-_free)/_size;
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::output
//       Access: Published
//  Description: ...intended for debugging only.
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
output(ostream &out) const {
  out << "UniqueIdAllocator(" << _min << ", " << _max << "), "
      << _free << " id's remaining of " << _size;
}

////////////////////////////////////////////////////////////////////
//     Function: UniqueIdAllocator::write
//       Access: Published
//  Description: ...intended for debugging only.
////////////////////////////////////////////////////////////////////
void UniqueIdAllocator::
write(ostream &out) const {
  out << "_min: " << _min << "; _max: " << _max
      << ";\n_next_free: " << PN_int32(_next_free)
      << "; _last_free: " << PN_int32(_last_free)
      << "; _size: " << _size
      << "; _free: " << _free
      << "; used: " << _size - _free
      << "; fraction_used: " << fraction_used()
      << ";\n";

  out << "Table:";
  for (PN_uint32 i = 0; i < _size; ++i) {
    out << " " << PN_int32(_table[i]);
  }
  out << "\n";

  out << "Free chain:";
  PN_uint32 index = _next_free;
  while (index != IndexEnd) {
    out << " " << index + _min;
    index = _table[index];
  }
  out << "\n";
}

