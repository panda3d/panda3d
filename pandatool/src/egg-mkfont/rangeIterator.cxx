// Filename: rangeIterator.cxx
// Created by:  drose (07Sep03)
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

#include "rangeIterator.h"

////////////////////////////////////////////////////////////////////
//     Function: RangeIterator::Constructor
//       Access: Public
//  Description: Constructs an iterator to walk through the codes on
//               the descriptor.  It is important not to modify the
//               RangeDescription object during the lifetime of the
//               iterator.
////////////////////////////////////////////////////////////////////
RangeIterator::
RangeIterator(const RangeDescription &desc) :
  _desc(desc) 
{
  _it = _desc._range_list.begin();
  if (_it == _desc._range_list.end()) {
    _code = -1;
  } else {
    _code = (*_it)._from_code;
    _codes_generated.insert(_code);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RangeIterator::next
//       Access: Public
//  Description: Advances the iterator to the next code.  Returns true
//               if there is a next code, or false if there are no
//               mode codes.
////////////////////////////////////////////////////////////////////
bool RangeIterator::
next() {
  do {
    if (_it == _desc._range_list.end()) {
      return false;
    }

    if (_code < (*_it)._to_code) {
      _code++;

    } else {
      _it++;
      if (_it == _desc._range_list.end()) {
        _code = -1;
        return false;
      }

      _code = (*_it)._from_code;
    }

    // If this code has already been generated, repeat and skip to the
    // next one.
  } while (!_codes_generated.insert(_code).second);

  return true;
}
