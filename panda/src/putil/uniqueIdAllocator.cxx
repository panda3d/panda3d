/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file uniqueIdAllocator.cxx
 * @author schuyler
 * @date 2003-03-13
 */

#include "pandabase.h"
#include "pnotify.h"
#include "notifyCategoryProxy.h"

#include "uniqueIdAllocator.h"

using std::endl;

NotifyCategoryDecl(uniqueIdAllocator, EXPCL_PANDA_PUTIL, EXPTP_PANDA_PUTIL);
NotifyCategoryDef(uniqueIdAllocator, "");

const uint32_t UniqueIdAllocator::IndexEnd = (uint32_t)-1;
const uint32_t UniqueIdAllocator::IndexAllocated = (uint32_t)-2;

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

/**
 * Create a free id pool in the range [min:max].
 */
UniqueIdAllocator::
UniqueIdAllocator(uint32_t min, uint32_t max)
  : _min(min), _max(max) {
  uniqueIdAllocator_debug("UniqueIdAllocator("<<min<<", "<<max<<")");

  nassertv(_max >= _min);
  _size = _max-_min+1; // +1 because min and max are inclusive.
  nassertv(_size != 0); // size must be > 0.

  _table = (uint32_t *)PANDA_MALLOC_ARRAY(_size * sizeof(uint32_t));
  nassertv(_table); // This should be redundant if new throws an exception.

  for (uint32_t i = 0; i < _size; ++i) {
    _table[i] = i + 1;
  }
  _table[_size - 1] = IndexEnd;
  _next_free = 0;
  _last_free = _size - 1;
  _free = _size;
}

/**
 *
 */
UniqueIdAllocator::
~UniqueIdAllocator() {
  uniqueIdAllocator_debug("~UniqueIdAllocator()");
  PANDA_FREE_ARRAY(_table);
}


/**
 * Returns an id between _min and _max (that were passed to the constructor).
 * IndexEnd is returned if no ids are available.
 */
uint32_t UniqueIdAllocator::
allocate() {
  if (_next_free == IndexEnd) {
    // ...all ids allocated.
    uniqueIdAllocator_warning("allocate Error: no more free ids.");
    return IndexEnd;
  }
  uint32_t index = _next_free;
  nassertr(_table[index] != IndexAllocated, IndexEnd);

  _next_free = _table[_next_free];
  _table[index] = IndexAllocated;

  --_free;

  uint32_t id = index + _min;
  uniqueIdAllocator_debug("allocate() returning " << id);
  return id;
}

/**
 * This may be called to mark a particular id as having already been allocated
 * (for instance, by a prior pass).  The specified id is removed from the
 * available pool.
 *
 * Because of the limitations of this algorithm, this is most efficient when
 * it is called before the first call to allocate(), and when all the calls to
 * initial_reserve_id() are made in descending order by id.  However, this is
 * a performance warning only; if performance is not an issue, any id may be
 * reserved at any time.
 */
void UniqueIdAllocator::
initial_reserve_id(uint32_t id) {
  nassertv(id >= _min && id <= _max); // Attempt to reserve out-of-range id.
  uint32_t index = id - _min; // Convert to _table index.

  nassertv(_table[index] != IndexAllocated);

  if (_free == 1) {
    // We just reserved the last element in the free chain.
    _next_free = IndexEnd;

  } else if (_next_free == index) {
    // We're reserving the head of the free chain.
    _next_free = _table[index];

  } else {
    // Since we don't store back pointers in the free chain, we have to search
    // for the element in the free chain that points to this index.

/*
 * However, there is an easy optimal case: because we expect that this call
 * will be made before any calls to allocate(), hopefully is it still true
 * that the _table is still set up such that _table[i] = i+1 (and if the
 * numbers are reserved in descending order, this will be true at least for
 * all i <= index).  Thus, the free link to slot [index] is expected to be the
 * slot right before it, or if not, it usually won't be far before it.
 */

    uint32_t prev_index = index;
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

/**
 * Checks the allocated state of an index. Returns true for
 * indices that are currently allocated and in use.
 */
bool UniqueIdAllocator::
is_allocated(uint32_t id) {
  if (id < _min || id > _max) {
    // This id is out of range, not allocated.
    return false;
  }

  uint32_t index = id - _min; // Convert to _table index.
  return _table[index] == IndexAllocated;
}

/**
 * Free an allocated index (index must be between _min and _max that were
 * passed to the constructor).
 */
void UniqueIdAllocator::
free(uint32_t id) {
  uniqueIdAllocator_debug("free("<<id<<")");

  nassertv(id >= _min && id <= _max); // Attempt to free out-of-range id.
  uint32_t index = id - _min; // Convert to _table index.
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


/**
 * return the decimal fraction of the pool that is used.  The range is 0 to
 * 1.0 (e.g.  75% would be 0.75).
 */
PN_stdfloat UniqueIdAllocator::
fraction_used() const {
  return PN_stdfloat(_size-_free)/_size;
}

/**
 * ...intended for debugging only.
 */
void UniqueIdAllocator::
output(std::ostream &out) const {
  out << "UniqueIdAllocator(" << _min << ", " << _max << "), "
      << _free << " id's remaining of " << _size;
}

/**
 * ...intended for debugging only.
 */
void UniqueIdAllocator::
write(std::ostream &out) const {
  out << "_min: " << _min << "; _max: " << _max
      << ";\n_next_free: " << int32_t(_next_free)
      << "; _last_free: " << int32_t(_last_free)
      << "; _size: " << _size
      << "; _free: " << _free
      << "; used: " << _size - _free
      << "; fraction_used: " << fraction_used()
      << ";\n";

  out << "Table:";
  for (uint32_t i = 0; i < _size; ++i) {
    out << " " << int32_t(_table[i]);
  }
  out << "\n";

  out << "Free chain:";
  uint32_t index = _next_free;
  while (index != IndexEnd) {
    out << " " << index + _min;
    index = _table[index];
  }
  out << "\n";
}
