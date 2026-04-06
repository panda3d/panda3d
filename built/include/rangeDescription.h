/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rangeDescription.h
 * @author drose
 * @date 2003-09-07
 */

#ifndef RANGEDESCRIPTION_H
#define RANGEDESCRIPTION_H

#include "pandatoolbase.h"
#include "pvector.h"

/**
 * This describes a sparse range of Unicode character codes for conversion
 * that may be specified on the command line.
 */
class RangeDescription {
public:
  RangeDescription();

  bool parse_parameter(const std::string &param);
  INLINE void add_singleton(int code);
  INLINE void add_range(int from_code, int to_code);
  INLINE bool is_empty() const;

  void output(std::ostream &out) const;

private:
  bool parse_word(const std::string &word);
  bool parse_code(const std::string &word, int &code);
  bool parse_bracket(const std::string &str);

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

INLINE std::ostream &operator << (std::ostream &out, const RangeDescription &range);

#include "rangeDescription.I"

#endif
