// Filename: config_egg.cxx
// Created by:  drose (19Mar00)
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

#include "config_egg.h"
#include "eggRenderMode.h"
#include "eggAnimData.h"
#include "eggAttributes.h"
#include "eggBin.h"
#include "eggBinMaker.h"
#include "eggComment.h"
#include "eggCoordinateSystem.h"
#include "eggCurve.h"
#include "eggExternalReference.h"
#include "eggFilenameNode.h"
#include "eggGroup.h"
#include "eggGroupNode.h"
#include "eggLine.h"
#include "eggMaterial.h"
#include "eggNameUniquifier.h"
#include "eggNamedObject.h"
#include "eggNode.h"
#include "eggNurbsCurve.h"
#include "eggNurbsSurface.h"
#include "eggObject.h"
#include "eggPoint.h"
#include "eggPolygon.h"
#include "eggPoolUniquifier.h"
#include "eggPrimitive.h"
#include "eggSAnimData.h"
#include "eggSurface.h"
#include "eggSwitchCondition.h"
#include "eggTable.h"
#include "eggTexture.h"
#include "eggUserData.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggXfmAnimData.h"
#include "eggXfmSAnim.h"

#include "dconfig.h"
#include "get_config_path.h"

Configure(config_egg);
NotifyCategoryDef(egg, "");

ConfigureFn(config_egg) {
  init_libegg();
}

const DSearchPath &
get_egg_path() {
  static DSearchPath *egg_path = NULL;
  return get_config_path("egg-path", egg_path);
}

// Set this true to support loading of old character animation files, which
// had the convention that the order "phr" implied a reversed roll.
bool egg_support_old_anims = config_egg.GetBool("egg-support-old-anims", true);

////////////////////////////////////////////////////////////////////
//     Function: init_libegg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libegg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  EggRenderMode::init_type();
  EggAnimData::init_type();
  EggAttributes::init_type();
  EggBin::init_type();
  EggBinMaker::init_type();
  EggComment::init_type();
  EggCoordinateSystem::init_type();
  EggCurve::init_type();
  EggData::init_type();
  EggExternalReference::init_type();
  EggFilenameNode::init_type();
  EggGroup::init_type();
  EggGroupNode::init_type();
  EggLine::init_type();
  EggMaterial::init_type();
  EggNameUniquifier::init_type();
  EggNamedObject::init_type();
  EggNode::init_type();
  EggNurbsCurve::init_type();
  EggNurbsSurface::init_type();
  EggObject::init_type();
  EggPoint::init_type();
  EggPolygon::init_type();
  EggPoolUniquifier::init_type();
  EggPrimitive::init_type();
  EggSAnimData::init_type();
  EggSurface::init_type();
  EggSwitchCondition::init_type();
  EggSwitchConditionDistance::init_type();
  EggTable::init_type();
  EggTexture::init_type();
  EggUserData::init_type();
  EggVertex::init_type();
  EggVertexPool::init_type();
  EggXfmAnimData::init_type();
  EggXfmSAnim::init_type();
}
