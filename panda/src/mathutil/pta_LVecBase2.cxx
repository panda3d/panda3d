/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LVecBase2.cxx
 * @author drose
 * @date 2010-02-27
 */

#include "pta_LVecBase2.h"

template class PointerToBase<ReferenceCountedVector<LVecBase2f> >;
template class PointerToArrayBase<LVecBase2f>;
template class PointerToArray<LVecBase2f>;
template class ConstPointerToArray<LVecBase2f>;

template class PointerToBase<ReferenceCountedVector<LVecBase2d> >;
template class PointerToArrayBase<LVecBase2d>;
template class PointerToArray<LVecBase2d>;
template class ConstPointerToArray<LVecBase2d>;

template class PointerToBase<ReferenceCountedVector<LVecBase2i> >;
template class PointerToArrayBase<LVecBase2i>;
template class PointerToArray<LVecBase2i>;
template class ConstPointerToArray<LVecBase2i>;
