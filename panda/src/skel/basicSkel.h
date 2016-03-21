/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file basicSkel.h
 * @author jyelon
 * @date 2007-01-31
 */

#ifndef BASICSKEL_H
#define BASICSKEL_H

#include "pandabase.h"

/**
 * This is the most basic of the skeleton classes.  It stores an integer, and
 * will return it on request.
 *
 * The skeleton classes are intended to help you learn how to add C++ classes
 * to panda.  See also the manual, "Adding C++ Classes to Panda."
 */
class EXPCL_PANDASKEL BasicSkel {
PUBLISHED:
  INLINE BasicSkel();
  INLINE ~BasicSkel();

  // These inline functions allow you to get and set _value.
  INLINE void set_value(int n);
  INLINE int  get_value();

  // These do the same thing as the functions above.
  void set_value_alt(int n);
  int  get_value_alt();

private:
  int _value;
};

#include "basicSkel.I"

#endif
