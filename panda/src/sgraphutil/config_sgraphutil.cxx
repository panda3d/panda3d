// Filename: config_sgraphutil.cxx
// Created by:  drose (21Feb00)
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

#include "config_sgraphutil.h"
#include "directRenderTraverser.h"

#include <dconfig.h>

Configure(config_sgraphutil);
NotifyCategoryDef(sgraphutil, "");

ConfigureFn(config_sgraphutil) {
  DirectRenderTraverser::init_type();
}

// Set this true to enable simple view-frustum culling: the
// elimination of branches of the scene graph from the drawing
// pipeline, based on the intersection test of its bounding sphere
// with the viewing frustum.  Usually you'd only want to turn it off
// if it misbehaved, although there are rare cases in whice the
// culling logic is more expensive than the cost of drawing more than
// you can see.
const bool view_frustum_cull = config_sgraphutil.GetBool("view-frustum-cull", true);

// Set this to color everything outside of the frustum red instead of
// culling it.  Presumably this is only useful for debugging culling.
const bool fake_view_frustum_cull = config_sgraphutil.GetBool("fake-view-frustum-cull", false);

// Setting this true causes the app traversal to be done during the
// draw traversal, instead of explicitly when the AppTraverser is
// called.
const bool implicit_app_traversal = config_sgraphutil.GetBool("implicit-app-traversal", true);

