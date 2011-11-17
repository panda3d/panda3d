// Filename: pandaNodeChain.I
// Created by:  drose (21Apr06)
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
//     Function: PandaNodeChain::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNodeChain::
PandaNodeChain(const char *lock_name) :
  LinkedListNode(true),  // This object is the root of a list of PandaNodes.
 _lock(lock_name)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PandaNodeChain::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE PandaNodeChain::
~PandaNodeChain() {
  _next = NULL;
  _prev = NULL;
}
