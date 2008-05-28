// Filename: config_lwo.cxx
// Created by:  drose (23Apr01)
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

#include "config_lwo.h"
#include "iffChunk.h"
#include "iffGenericChunk.h"
#include "iffInputFile.h"
#include "lwoBoundingBox.h"
#include "lwoChunk.h"
#include "lwoClip.h"
#include "lwoDiscontinuousVertexMap.h"
#include "lwoGroupChunk.h"
#include "lwoHeader.h"
#include "lwoInputFile.h"
#include "lwoLayer.h"
#include "lwoPoints.h"
#include "lwoPolygons.h"
#include "lwoPolygonTags.h"
#include "lwoStillImage.h"
#include "lwoSurface.h"
#include "lwoSurfaceBlock.h"
#include "lwoSurfaceBlockAxis.h"
#include "lwoSurfaceBlockChannel.h"
#include "lwoSurfaceBlockCoordSys.h"
#include "lwoSurfaceBlockEnabled.h"
#include "lwoSurfaceBlockImage.h"
#include "lwoSurfaceBlockOpacity.h"
#include "lwoSurfaceBlockProjection.h"
#include "lwoSurfaceBlockHeader.h"
#include "lwoSurfaceBlockRefObj.h"
#include "lwoSurfaceBlockRepeat.h"
#include "lwoSurfaceBlockTMap.h"
#include "lwoSurfaceBlockTransform.h"
#include "lwoSurfaceBlockVMapName.h"
#include "lwoSurfaceBlockWrap.h"
#include "lwoSurfaceColor.h"
#include "lwoSurfaceParameter.h"
#include "lwoSurfaceSidedness.h"
#include "lwoSurfaceSmoothingAngle.h"
#include "lwoTags.h"
#include "lwoVertexMap.h"

#include "dconfig.h"

Configure(config_lwo);

ConfigureFn(config_lwo) {
  init_liblwo();
}

////////////////////////////////////////////////////////////////////
//     Function: init_liblwo
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_liblwo() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  IffChunk::init_type();
  IffGenericChunk::init_type();
  IffInputFile::init_type();
  LwoBoundingBox::init_type();
  LwoChunk::init_type();
  LwoClip::init_type();
  LwoDiscontinuousVertexMap::init_type();
  LwoGroupChunk::init_type();
  LwoHeader::init_type();
  LwoInputFile::init_type();
  LwoLayer::init_type();
  LwoPoints::init_type();
  LwoPolygons::init_type();
  LwoPolygonTags::init_type();
  LwoTags::init_type();
  LwoStillImage::init_type();
  LwoSurface::init_type();
  LwoSurfaceBlock::init_type();
  LwoSurfaceBlockAxis::init_type();
  LwoSurfaceBlockChannel::init_type();
  LwoSurfaceBlockCoordSys::init_type();
  LwoSurfaceBlockEnabled::init_type();
  LwoSurfaceBlockImage::init_type();
  LwoSurfaceBlockOpacity::init_type();
  LwoSurfaceBlockProjection::init_type();
  LwoSurfaceBlockHeader::init_type();
  LwoSurfaceBlockRefObj::init_type();
  LwoSurfaceBlockRepeat::init_type();
  LwoSurfaceBlockTMap::init_type();
  LwoSurfaceBlockTransform::init_type();
  LwoSurfaceBlockVMapName::init_type();
  LwoSurfaceBlockWrap::init_type();
  LwoSurfaceColor::init_type();
  LwoSurfaceParameter::init_type();
  LwoSurfaceSidedness::init_type();
  LwoSurfaceSmoothingAngle::init_type();
  LwoVertexMap::init_type();
}

