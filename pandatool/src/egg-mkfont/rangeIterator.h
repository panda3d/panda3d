// Filename: rangeIterator.h
// Created by:  drose (07Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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

