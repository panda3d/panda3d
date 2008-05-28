// Filename: bamCacheIndex.I
// Created by:  drose (19Jun06)
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
//     Function: BamCacheIndex::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
INLINE BamCacheIndex::
BamCacheIndex() : 
  LinkedListNode(true),
  _cache_size(0)
{
}
