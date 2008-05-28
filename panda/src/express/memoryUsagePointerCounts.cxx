// Filename: memoryUsagePointerCounts.cxx
// Created by:  drose (04Jun01)
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

#include "memoryUsagePointerCounts.h"

#ifdef DO_MEMORY_USAGE

#include "memoryInfo.h"

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointerCounts::add_info
//       Access: Public
//  Description: Adds a pointer definition to the counter.
////////////////////////////////////////////////////////////////////
void MemoryUsagePointerCounts::
add_info(MemoryInfo *info) {
  _count++;

  if (info->is_size_known()) {
    _size += info->get_size();
  } else {
    _unknown_size_count++;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointerCounts::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void MemoryUsagePointerCounts::
output(ostream &out) const {
  out << _count << " pointers";
  if (_unknown_size_count < _count) {
    out << ", ";
    output_bytes(out, _size);
    out << ", avg ";
    output_bytes(out, _size / (_count - _unknown_size_count));
    out << " each";
      
    if (_unknown_size_count != 0) {
      out << " (" << _unknown_size_count << " of unknown size)";
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MemoryUsagePointerCounts::output_bytes
//       Access: Private, Static
//  Description: Formats a size in bytes in a meaningful and concise
//               way for output, with units.
////////////////////////////////////////////////////////////////////
void MemoryUsagePointerCounts::
output_bytes(ostream &out, size_t size) {
  if (size < 4 * 1024) {
    out << size << " bytes";
    
  } else if (size < 4 * 1024 * 1024) {
    out << size / 1024 << " Kb";
    
  } else {
    out << size / (1024 * 1024) << " Mb";
  }
}

#endif  // DO_MEMORY_USAGE

