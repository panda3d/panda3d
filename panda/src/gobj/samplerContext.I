// Filename: samplerContext.I
// Created by:  rdb (11Dec14)
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


////////////////////////////////////////////////////////////////////
//     Function: SamplerContext::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE SamplerContext::
SamplerContext(const SamplerState &sampler) :
  SimpleLruPage(1)
{
}
