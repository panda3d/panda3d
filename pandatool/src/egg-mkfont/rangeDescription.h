// Filename: rangeDescription.h
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

#ifndef RANGEDESCRIPTION_H
#define RANGEDESCRIPTION_H

#include "pandatoolbase.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : RangeDescription
// Description : This describes a sparse range of Unicode character
//               codes for conversion that may be specified on the
//               command line.
////////////////////////////////////////////////////////////////////
class RangeDescription {
public:
  RangeDescription();

  bool parse_parameter(const string &param);
  INLINE void add_singleton(int code);
  INLINE void add_range(int from_code, int to_code);
  INLINE bool is_empty() const;

  void output(ostream &out) const;

private:
  bool parse_word(const string &word);
  bool parse_code(const string &word, int &code);
  bool parse_bracket(const string &str);

private:
  class Range {
  public:
    INLINE Range(int code);
    INLINE Range(int from_code, int to_code);

    int _from_code;
    int _to_code;
  };

  typedef pvector<Range> RangeList;
  RangeList _range_list;

  friend class RangeIterator;
};

INLINE ostream &operator << (ostream &out, const RangeDescription &range);

#include "rangeDescription.I"

#endif

