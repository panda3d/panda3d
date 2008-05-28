// Filename: config_dgraph.cxx
// Created by:  drose (01Mar00)
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

#include "config_dgraph.h"
#include "dataNode.h"
#include "dataNodeTransmit.h"

#include "dconfig.h"

Configure(config_dgraph);
NotifyCategoryDef(dgraph, "");

ConfigureFn(config_dgraph) {
  DataNode::init_type();
  DataNodeTransmit::init_type();

  DataNodeTransmit::register_with_read_factory();
}
