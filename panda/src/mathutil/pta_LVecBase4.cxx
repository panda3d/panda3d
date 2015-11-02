// Filename: pta_LVecBase4.cxx
// Created by:  drose (27Feb10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pta_LVecBase4.h"

// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

template class PointerToBase<ReferenceCountedVector<UnalignedLVecBase4f> >;
template class PointerToArrayBase<UnalignedLVecBase4f>;
template class PointerToArray<UnalignedLVecBase4f>;
template class ConstPointerToArray<UnalignedLVecBase4f>;

template class PointerToBase<ReferenceCountedVector<UnalignedLVecBase4d> >;
template class PointerToArrayBase<UnalignedLVecBase4d>;
template class PointerToArray<UnalignedLVecBase4d>;
template class ConstPointerToArray<UnalignedLVecBase4d>;

template class PointerToBase<ReferenceCountedVector<UnalignedLVecBase4i> >;
template class PointerToArrayBase<UnalignedLVecBase4i>;
template class PointerToArray<UnalignedLVecBase4i>;
template class ConstPointerToArray<UnalignedLVecBase4i>;
