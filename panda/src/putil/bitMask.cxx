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

template class BitMask<uint16_t, 16>;
template class BitMask<uint32_t, 32>;
template class BitMask<uint64_t, 64>;
