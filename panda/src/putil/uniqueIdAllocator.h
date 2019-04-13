/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file uniqueIdAllocator.h
 * @author schuyler
 * @date 2003-03-13
 */

#ifndef _UNIQUEIDALLOCATOR_H //[
#define _UNIQUEIDALLOCATOR_H

#include "pandabase.h"
#include "numeric_types.h"

/**
 * Manage a set of ID values from min to max inclusive.  The ID numbers that
 * are freed will be allocated (reused) in the same order.  I.e.  the oldest
 * ID numbers will be allocated.
 *
 * This implementation will use 4 bytes per id number, plus a few bytes of
 * management data.  e.g.  10,000 ID numbers will use 40KB.
 *
 * Also be advised that ID -1 and -2 are used internally by the allocator.  If
 * allocate returns IndexEnd (-1) then the allocator is out of free ID
 * numbers.
 *
 * There are other implementations that can better leverage runs of used or
 * unused IDs or use bit arrays for the IDs.  But, it takes extra work to
 * track the age of freed IDs, which is required for what we wanted.  If you
 * would like to kick around other implementation ideas, please contact
 * Schuyler.
 */
class EXPCL_PANDA_PUTIL UniqueIdAllocator {
PUBLISHED:
  explicit UniqueIdAllocator(uint32_t min=0, uint32_t max=20);
  ~UniqueIdAllocator();

  uint32_t allocate();
  void initial_reserve_id(uint32_t id);

  bool is_allocated(uint32_t index);

  void free(uint32_t index);
  PN_stdfloat fraction_used() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out) const;

public:
  static const uint32_t IndexEnd;
  static const uint32_t IndexAllocated;

protected:
  // _table stores an array of _size words, corresponding to each allocatable
  // id.

  // For each free id, the table entry at the corresponding index contains the
  // index number of the next free id, thus defining a chain of free id's.
  // The last element of the chain contains IndexEnd.

  // For an allocated id, the table entry at the corresponding index contains
  // IndexAllocated.
  uint32_t *_table;

  // The minimum and maximum as passed to the constructor.
  uint32_t _min;
  uint32_t _max;

  // This is the head of the free chain: as elements are allocated, they are
  // taken from _table[_next_free].  If the free chain is empty, _next_free ==
  // IndexEnd.
  uint32_t _next_free;

  // This is the tail of the free chain: as elements are freed, they are
  // stored in _table[_last_free].  Normally, _table[_last_free] is the end of
  // the free chain, unless the free chain is empty.
  uint32_t _last_free;

  // The total number of elements in _table.
  uint32_t _size;

  // The number of free elements in _table.
  uint32_t _free;
};

#endif //]
