/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pgMouseWatcherParameter.I
 * @author drose
 * @date 2001-07-05
 */

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherParameter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGMouseWatcherParameter::
PGMouseWatcherParameter() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherParameter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PGMouseWatcherParameter::
PGMouseWatcherParameter(const MouseWatcherParameter &copy) :
  MouseWatcherParameter(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PGMouseWatcherParameter::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PGMouseWatcherParameter::
operator = (const MouseWatcherParameter &copy) {
  MouseWatcherParameter::operator = (copy);
}
