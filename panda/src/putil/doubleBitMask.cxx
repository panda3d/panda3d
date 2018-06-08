/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file doubleBitMask.cxx
 * @author drose
 * @date 2000-06-08
 */

#include "doubleBitMask.h"

template class DoubleBitMask<BitMaskNative>;
template class DoubleBitMask<DoubleBitMaskNative>;

#if !defined(CPPPARSER) && !defined(__APPLE__)
#include <type_traits>

static_assert(std::is_literal_type<DoubleBitMaskNative>::value, "DoubleBitMaskNative is not a literal type");
static_assert(std::is_literal_type<QuadBitMaskNative>::value, "QuadBitMaskNative is not a literal type");
#endif
