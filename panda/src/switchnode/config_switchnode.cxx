// Filename: config_switchnode.cxx
// Created by:  drose (19Jan00)
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

#include "config_switchnode.h"
#include "LODNode.h"
#include "sequenceNode.h"

#include <dconfig.h>

Configure(config_switchnode);
NotifyCategoryDef(switchnode, "");

ConfigureFn(config_switchnode) {
  LODNode::init_type();
  SequenceNode::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  LODNode::register_with_read_factory();
  SequenceNode::register_with_read_factory();
}
