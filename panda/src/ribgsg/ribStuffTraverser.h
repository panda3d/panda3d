// Filename: ribStuffTraverser.h
// Created by:  drose (16Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef RIBSTUFFTRAVERSER_H
#define RIBSTUFFTRAVERSER_H

#include "pandabase.h"

#include <traverserVisitor.h>
#include <renderRelation.h>
#include <allAttributesWrapper.h>
#include <allTransitionsWrapper.h>
#include <nullLevelState.h>

class RIBGraphicsStateGuardian;

////////////////////////////////////////////////////////////////////
//       Class : RibStuffTraverser
// Description :
////////////////////////////////////////////////////////////////////
class RibStuffTraverser :
  public TraverserVisitor<AllTransitionsWrapper, NullLevelState> {
public:
  RibStuffTraverser(RIBGraphicsStateGuardian *gsg) : _gsg(gsg) { }
  bool reached_node(Node *node, AllAttributesWrapper &state,
                    NullLevelState &);

public:
  RIBGraphicsStateGuardian *_gsg;
};

#endif

