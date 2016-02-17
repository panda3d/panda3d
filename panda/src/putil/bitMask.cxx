/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file bitMask.cxx
 * @author drose
 * @date 2000-06-08
 */

#include "bitMask.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

template class BitMask<PN_uint16, 16>;
template class BitMask<PN_uint32, 32>;
template class BitMask<PN_uint64, 64>;
