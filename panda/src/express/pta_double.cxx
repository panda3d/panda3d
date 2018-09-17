/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_double.cxx
 * @author drose
 * @date 2000-05-10
 */

#include "pta_double.h"

template class PointerToBase<ReferenceCountedVector<double> >;
template class PointerToArrayBase<double>;
template class PointerToArray<double>;
template class ConstPointerToArray<double>;
