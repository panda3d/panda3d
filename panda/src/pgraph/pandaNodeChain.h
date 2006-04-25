// Filename: pandaNodeChain.h
// Created by:  drose (21Apr06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef PANDANODECHAIN_H
#define PANDANODECHAIN_H

#include "pandabase.h"
#include "linkedListNode.h"
#include "pmutex.h"

class PandaNode;

////////////////////////////////////////////////////////////////////
//       Class : PandaNodeChain
// Description : This class maintains a linked list of PandaNodes.
//               It's used to maintain a list of PandaNodes whose
//               _prev_transform is different from their _transform
//               (in pipeline stage 0).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PandaNodeChain : private LinkedListNode {
public:
  INLINE PandaNodeChain();
  INLINE ~PandaNodeChain();

  Mutex _lock;

  friend class PandaNode;
};

#include "pandaNodeChain.I"

#endif

