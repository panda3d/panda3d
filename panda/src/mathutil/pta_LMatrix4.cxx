/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pta_LMatrix4.cxx
 * @author drose
 * @date 2010-02-27
 */

#include "pta_LMatrix4.h"

template class PointerToBase<ReferenceCountedVector<UnalignedLMatrix4f> >;
template class PointerToArrayBase<UnalignedLMatrix4f>;
template class PointerToArray<UnalignedLMatrix4f>;
template class ConstPointerToArray<UnalignedLMatrix4f>;

template class PointerToBase<ReferenceCountedVector<UnalignedLMatrix4d> >;
template class PointerToArrayBase<UnalignedLMatrix4d>;
template class PointerToArray<UnalignedLMatrix4d>;
template class ConstPointerToArray<UnalignedLMatrix4d>;
