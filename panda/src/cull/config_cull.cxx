// Filename: config_cull.cxx
// Created by:  drose (07Apr00)
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


#include "config_cull.h"
#include "cullTraverser.h"
#include "geomBin.h"
#include "geomBinUnsorted.h"
#include "geomBinBackToFront.h"
#include "geomBinGroup.h"
#include "geomBinNormal.h"
#include "geomBinTransition.h"
#include "geomBinAttribute.h"
#include "geomBinFixed.h"
#include "directRenderTransition.h"

#include <dconfig.h>

ConfigureDef(config_cull);
NotifyCategoryDef(cull, "");

ConfigureFn(config_cull) {
  CullTraverser::init_type();
  GeomBin::init_type();
  GeomBinUnsorted::init_type();
  GeomBinBackToFront::init_type();
  GeomBinGroup::init_type();
  GeomBinNormal::init_type();
  GeomBinTransition::init_type();
  GeomBinAttribute::init_type();
  GeomBinFixed::init_type();
  DirectRenderTransition::init_type();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  GeomBinTransition::register_with_read_factory();
}

// Set this true to force all of the caching to blow itself away every
// frame.  Normally you would only do this during testing.
const bool cull_force_update = config_cull.GetBool("cull-force-update", false);

