/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file randomizer.h
 * @author drose
 * @date 2007-01-18
 */

#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include "pandabase.h"
#include "mersenne.h"

#include <time.h>
#include <math.h>

/**
 * A handy class to return random numbers.
 */
class EXPCL_PANDA_MATHUTIL Randomizer {
PUBLISHED:
  INLINE explicit Randomizer(unsigned long seed = 0);
  INLINE Randomizer(const Randomizer &copy);
  INLINE void operator = (const Randomizer &copy);

  INLINE int random_int(int range);
  INLINE double random_real(double range);
  INLINE double random_real_unit();

  INLINE static unsigned long get_next_seed();
  INLINE unsigned long get_seed();

private:
  Mersenne _mersenne;
  static Mersenne _next_seed;
  static bool _got_first_seed;
};

#include "randomizer.I"

#endif
