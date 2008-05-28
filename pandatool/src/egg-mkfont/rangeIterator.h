// Filename: rangeIterator.h
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

#ifndef RANGEITERATOR_H
#define RANGEITERATOR_H

#include "pandatoolbase.h"
#include "rangeDescription.h"

#include "pset.h"

////////////////////////////////////////////////////////////////////
//       Class : RangeIterator
// Description : Walks through all the Unicode characters described by
//               a RangeDescription class.
////////////////////////////////////////////////////////////////////
class RangeIterator {
public:
  RangeIterator(const RangeDescription &desc);

  INLINE int get_code() const;
  bool next();
  INLINE bool eof() const;

private:
  const RangeDescription &_desc;
  RangeDescription::RangeList::const_iterator _it;
  int _code;

  typedef pset<int> Codes;
  Codes _codes_generated;
};

#include "rangeIterator.I"

#endif

