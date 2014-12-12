// Filename: samplerContext.cxx
// Created by:  rdb (11Dec14))
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

#include "samplerContext.h"

TypeHandle SamplerContext::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SamplerContext::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void SamplerContext::
output(ostream &out) const {
  SavedContext::output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: SamplerContext::write
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void SamplerContext::
write(ostream &out, int indent_level) const {
  SavedContext::write(out, indent_level);
}

