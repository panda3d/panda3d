// Filename: dcNumericRange.I
// Created by:  drose (21Jun04)
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


////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE DCNumericRange<NUM>::
DCNumericRange() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE DCNumericRange<NUM>::
DCNumericRange(const DCNumericRange<NUM> &copy) :
  _ranges(copy._ranges)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::Copy Assignment Operator
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE void DCNumericRange<NUM>::
operator = (const DCNumericRange<NUM> &copy) {
  _ranges = copy._ranges;
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::is_in_range
//       Access: Public
//  Description: Returns true if the indicated number is within the
//               specified range, false otherwise.
////////////////////////////////////////////////////////////////////
template <class NUM>
bool DCNumericRange<NUM>::
is_in_range(Number num) const {
  if (_ranges.empty()) {
    return true;
  }

  TYPENAME Ranges::const_iterator ri;
  for (ri = _ranges.begin(); ri != _ranges.end(); ++ri) {
    if (num >= (*ri)._min && num <= (*ri)._max) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::validate
//       Access: Public
//  Description: Convenience function to validate the indicated
//               number.  If the number is within the specified range,
//               does nothing; otherwise, if it is outside the range,
//               sets validation_error to true.
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE void DCNumericRange<NUM>::
validate(Number num, bool &validation_error) const {
  if (!is_in_range(num)) {
    validation_error = true;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class NUM>
void DCNumericRange<NUM>::
output(ostream &out, Number divisor) const {
  if (!_ranges.empty()) {
    TYPENAME Ranges::const_iterator ri;
    ri = _ranges.begin();
    output_minmax(out, divisor, *ri);
    ++ri;
    while (ri != _ranges.end()) {
      out << ", ";
      output_minmax(out, divisor, *ri);
      ++ri;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::clear
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE void DCNumericRange<NUM>::
clear() {
  _ranges.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::add_range
//       Access: Public
//  Description: Adds a new minmax to the list of ranges.  This is
//               normally called only during dc file parsing.  Returns
//               true if successful, or false if the new minmax
//               overlaps an existing minmax.
////////////////////////////////////////////////////////////////////
template <class NUM>
bool DCNumericRange<NUM>::
add_range(Number min, Number max) {
  // Check for an overlap.  This is probably indicative of a typo and
  // should be reported.
  if (max < min) {
    return false;
  }

  TYPENAME Ranges::const_iterator ri;
  for (ri = _ranges.begin(); ri != _ranges.end(); ++ri) {
    if ((min >= (*ri)._min && min <= (*ri)._max) ||
        (max >= (*ri)._min && max <= (*ri)._max) ||
        (min < (*ri)._min && max > (*ri)._max)) {
      return false;
    }
  }

  MinMax minmax;
  minmax._min = min;
  minmax._max = max;
  _ranges.push_back(minmax);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::is_empty
//       Access: Private
//  Description: Returns true if the range contains no elements (and
//               thus allows all numbers), false if it contains at
//               least one.
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE bool DCNumericRange<NUM>::
is_empty() const {
  return _ranges.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::get_num_ranges
//       Access: Private
//  Description: Returns the number of minmax components in the range
//               description.
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE int DCNumericRange<NUM>::
get_num_ranges() const {
  return _ranges.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::get_min
//       Access: Private
//  Description: Returns the minimum value defined by the nth component.
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE TYPENAME DCNumericRange<NUM>::Number DCNumericRange<NUM>::
get_min(int n) const {
  nassertr(n >= 0 && n < (int)_ranges.size(), 0);
  return _ranges[n]._min;
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::get_max
//       Access: Private
//  Description: Returns the maximum value defined by the nth component.
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE TYPENAME DCNumericRange<NUM>::Number DCNumericRange<NUM>::
get_max(int n) const {
  nassertr(n >= 0 && n < (int)_ranges.size(), 0);
  return _ranges[n]._max;
}

////////////////////////////////////////////////////////////////////
//     Function: DCNumericRange::output_minmax
//       Access: Private
//  Description: Outputs a single element of the range description.
////////////////////////////////////////////////////////////////////
template <class NUM>
INLINE void DCNumericRange<NUM>::
output_minmax(ostream &out, Number divisor, const MinMax &range) const {
  if (range._min == range._max) {
    out << (double)range._min / (double)divisor;
  } else {
    out << (double)range._min / (double)divisor
        << "-"
        << (double)range._max / (double)divisor;
  }
}
