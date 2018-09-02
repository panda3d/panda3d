/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dcNumericRange.h
 * @author drose
 * @date 2004-06-21
 */

#ifndef DCNUMERICRANGE_H
#define DCNUMERICRANGE_H

#include "dcbase.h"
#include "hashGenerator.h"
#include "dcPacker.h"

/**
 * Represents a range of legal integer or floating-point values.  This is used
 * to constrain simple numeric types, as well as array sizes.
 */
template <class NUM>
class EXPCL_DIRECT_DCPARSER DCNumericRange {
public:
  typedef NUM Number;

  INLINE DCNumericRange();
  INLINE DCNumericRange(Number min, Number max);
  INLINE DCNumericRange(const DCNumericRange &copy);
  INLINE void operator = (const DCNumericRange &copy);

  INLINE bool is_in_range(Number num) const;
  INLINE void validate(Number num, bool &range_error) const;

  INLINE bool has_one_value() const;
  INLINE Number get_one_value() const;

  INLINE void generate_hash(HashGenerator &hashgen) const;

  INLINE void output(std::ostream &out, Number divisor = 1) const;
  INLINE void output_char(std::ostream &out, Number divisor = 1) const;

public:
  INLINE void clear();
  INLINE bool add_range(Number min, Number max);

  INLINE bool is_empty() const;
  INLINE int get_num_ranges() const;
  INLINE Number get_min(int n) const;
  INLINE Number get_max(int n) const;

private:
  class MinMax {
  public:
    INLINE bool operator < (const MinMax &other) const;

    Number _min;
    Number _max;
  };
  INLINE void output_minmax(std::ostream &out, Number divisor, const MinMax &range) const;
  INLINE void output_minmax_char(std::ostream &out, const MinMax &range) const;

  typedef pvector<MinMax> Ranges;
  Ranges _ranges;
};

#include "dcNumericRange.I"

typedef DCNumericRange<int> DCIntRange;
typedef DCNumericRange<unsigned int> DCUnsignedIntRange;
typedef DCNumericRange<int64_t> DCInt64Range;
typedef DCNumericRange<uint64_t> DCUnsignedInt64Range;
typedef DCNumericRange<double> DCDoubleRange;

#endif
