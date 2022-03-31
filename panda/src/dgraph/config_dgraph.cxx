/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_dgraph.cxx
 * @author drose
 * @date 2000-03-01
 */

#include "config_dgraph.h"
#include "dataNode.h"
#include "dataNodeTransmit.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_DGRAPH)
  #error Buildsystem error: BUILDING_PANDA_DGRAPH not defined
#endif

Configure(config_dgraph);
NotifyCategoryDef(dgraph, "");

ConfigureFn(config_dgraph) {
  DataNode::init_type();
  DataNodeTransmit::init_type();

  DataNodeTransmit::register_with_read_factory();
}
