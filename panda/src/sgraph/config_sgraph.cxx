// Filename: config_sgraph.cxx
// Created by:  drose (12Apr00)
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

#include "config_sgraph.h"
#include "renderTraverser.h"
#include "geomNode.h"
#include "camera.h"
#include "planeNode.h"
#include "modelNode.h"
#include "modelRoot.h"
#include "lensNode.h"
#include "switchNode.h"

#include <dconfig.h>
#include <config_graph.h>

Configure(config_sgraph);
NotifyCategoryDef(sgraph, "");

ConfigureFn(config_sgraph) {
  init_libsgraph();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libsgraph
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libsgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  init_libgraph();

  RenderTraverser::init_type();
  GeomNode::init_type();
  Camera::init_type();
  PlaneNode::init_type();
  ModelNode::init_type();
  ModelRoot::init_type();
  LensNode::init_type();
  SwitchNode::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  GeomNode::register_with_read_factory();
  ModelNode::register_with_read_factory();
  ModelRoot::register_with_read_factory();
}
