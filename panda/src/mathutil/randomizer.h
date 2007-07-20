// Filename: randomizer.h
// Created by:  drose (18Jan07)
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

#ifndef RANDOMIZER_H
#define RANDOMIZER_H

#include "pandabase.h"
#include "mersenne.h"

////////////////////////////////////////////////////////////////////
//       Class : Randomizer
// Description : A handy class to return random numbers.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_MATHUTIL Randomizer {
public:
  INLINE Randomizer(unsigned long seed = 0);
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
