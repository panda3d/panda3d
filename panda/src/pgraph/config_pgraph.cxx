// Filename: config_pgraph.cxx
// Created by:  drose (21Feb02)
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

#include "config_pgraph.h"

#include "colorAttrib.h"
#include "qpgeomNode.h"
#include "pandaNode.h"
#include "renderAttrib.h"
#include "renderState.h"
#include "textureAttrib.h"
#include "transformAttrib.h"

#include "dconfig.h"

Configure(config_pgraph);
NotifyCategoryDef(pgraph, "");

ConfigureFn(config_pgraph) {
  init_libpgraph();
}


////////////////////////////////////////////////////////////////////
//     Function: init_libpgraph
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libpgraph() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  ColorAttrib::init_type();
  qpGeomNode::init_type();
  PandaNode::init_type();
  RenderAttrib::init_type();
  RenderState::init_type();
  TextureAttrib::init_type();
  TransformAttrib::init_type();

  ColorAttrib::register_with_read_factory();
  qpGeomNode::register_with_read_factory();
  PandaNode::register_with_read_factory();
  RenderState::register_with_read_factory();
  TextureAttrib::register_with_read_factory();
  TransformAttrib::register_with_read_factory();
}
