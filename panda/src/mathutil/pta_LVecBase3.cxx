/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LVecBase3.cxx
 * @author drose
 * @date 2010-02-27
 */

#include "pta_LVecBase3.h"

template class PointerToBase<ReferenceCountedVector<LVecBase3f> >;
template class PointerToArrayBase<LVecBase3f>;
template class PointerToArray<LVecBase3f>;
template class ConstPointerToArray<LVecBase3f>;

template class PointerToBase<ReferenceCountedVector<LVecBase3d> >;
template class PointerToArrayBase<LVecBase3d>;
template class PointerToArray<LVecBase3d>;
template class ConstPointerToArray<LVecBase3d>;

template class PointerToBase<ReferenceCountedVector<LVecBase3i> >;
template class PointerToArrayBase<LVecBase3i>;
template class PointerToArray<LVecBase3i>;
template class ConstPointerToArray<LVecBase3i>;
