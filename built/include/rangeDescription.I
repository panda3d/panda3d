/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file rangeDescription.I
 * @author drose
 * @date 2003-09-07
 */

/**
 *
 */
INLINE void RangeDescription::
add_singleton(int code) {
  _range_list.push_back(Range(code));
}

/**
 *
 */
INLINE void RangeDescription::
add_range(int from_code, int to_code) {
  _range_list.push_back(Range(from_code, to_code));
}

/**
 * Returns true if there are no codes described in the range.
 */
INLINE bool RangeDescription::
is_empty() const {
  return _range_list.empty();
}

/**
 *
 */
INLINE RangeDescription::Range::
Range(int code) :
  _from_code(code),
  _to_code(code)
{
}

/**
 *
 */
INLINE RangeDescription::Range::
Range(int from_code, int to_code) :
  _from_code(from_code),
  _to_code(to_code)
{
}

INLINE std::ostream &operator << (std::ostream &out, const RangeDescription &range) {
  range.output(out);
  return out;
}
