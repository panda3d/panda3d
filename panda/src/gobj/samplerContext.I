/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file samplerContext.I
 * @author rdb
 * @date 2014-12-11
 */

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
