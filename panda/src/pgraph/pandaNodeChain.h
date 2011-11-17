// Filename: pandaNodeChain.h
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

#ifndef PANDANODECHAIN_H
#define PANDANODECHAIN_H

#include "pandabase.h"
#include "linkedListNode.h"
#include "lightMutex.h"

class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : PandaNodeChain
// Description : This class maintains a linked list of PandaNodes.
//               It's used to maintain a list of PandaNodes whose
//               _prev_transform is different from their _transform
//               (in pipeline stage 0).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH PandaNodeChain : private LinkedListNode {
public:
  INLINE PandaNodeChain(const char *lock_name);
  INLINE ~PandaNodeChain();

  LightMutex _lock;

  friend class PandaNode;
};

#include "pandaNodeChain.I"

#endif

