/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file randomizer.cxx
 * @author drose
 * @date 2007-01-18
 */

#include "randomizer.h"

Mersenne Randomizer::_next_seed(0);
bool Randomizer::_got_first_seed = false;
