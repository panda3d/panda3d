// Filename: config_sgattrib.cxx
// Created by:  drose (10Mar00)
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

#include "config_sgattrib.h"
#include "renderRelation.h"
#include "colorTransition.h"
#include "textureTransition.h"
#include "decalTransition.h"
#include "depthTestTransition.h"
#include "depthWriteTransition.h"
#include "colorBlendTransition.h"
#include "renderModeTransition.h"
#include "materialTransition.h"
#include "texGenTransition.h"
#include "cullFaceTransition.h"
#include "colorMaskTransition.h"
#include "stencilTransition.h"
#include "textureApplyTransition.h"
#include "clipPlaneTransition.h"
#include "transparencyTransition.h"
#include "fogTransition.h"
#include "linesmoothTransition.h"
#include "transformTransition.h"
#include "texMatrixTransition.h"
#include "billboardTransition.h"
#include "polygonOffsetTransition.h"
#include "pruneTransition.h"
#include "drawBoundsTransition.h"
#include "pointShapeTransition.h"
#include "colorMatrixTransition.h"
#include "alphaTransformTransition.h"


#include <dconfig.h>

Configure(config_sgattrib);
NotifyCategoryDef(sgattrib, "");


// For performance testing reasons, it may be useful to support
// certain rendering special effects that require a special-case
// direct render traversal to varying less-than-complete degrees.  The
// variables support-decals, support-subrender, or support-direct may
// be set to change these.  support-decals specifically controls the
// rendering of decal geometry (polygons against a coplanar
// background), while support-subrender controls other effects such as
// LOD's and billboards.  support-direct controls any effect which
// requires switching to DirectRenderTraverser from a CullTraverser,
// including decals and things specifically flagged with a
// DirectRenderTransition.
//
// The legal values for each variable are:
//
//   on - This is the default, and causes the effect to be rendered
//        properly (if supported by the gsg backend).  This could have
//        performance implications in fill, transform, and
//        state-sorting.  This is equivalent to #t.
//
//  off - The special effect is ignored, and the geometry is rendered
//        as if the effect were not set at all.  The result will
//        generally be horrible looking, for instance with decals
//        Z-fighting with the base geometry.  This is equivalent to
//        #f.
//
// hide - The nested geometry--the decals, or the billboards, or
//        whatever--is not drawn at all.
//
// If compiled in NDEBUG mode, this variable is ignored and decals
// etc. are always on.
//
SupportDirect support_decals = SD_on;
SupportDirect support_subrender = SD_on;
SupportDirect support_direct = SD_on;

static SupportDirect
get_support_direct(const string &varname) {
  string type = config_sgattrib.GetString(varname, "");
  if (type == "on") {
    return SD_on;
  } else if (type == "off") {
    return SD_off;
  } else if (type == "hide") {
    return SD_hide;
  }

  // None of the above, so use #t/#f.
  if (config_sgattrib.GetBool(varname, true)) {
    return SD_on;
  } else {
    return SD_off;
  }
}

ConfigureFn(config_sgattrib) {
  support_decals = get_support_direct("support-decals");
  support_subrender = get_support_direct("support-subrender");
  support_direct = get_support_direct("support-direct");

  // MPG - we want to ensure that texture transitions are applied
  // before texgen transitions, so the texture transition must be
  // initialized first.

  RenderRelation::init_type();
  TextureTransition::init_type();
  TransformTransition::init_type();
  TexMatrixTransition::init_type();
  DecalTransition::init_type();
  DepthTestTransition::init_type();
  DepthWriteTransition::init_type();
  ColorBlendTransition::init_type();
  RenderModeTransition::init_type();
  MaterialTransition::init_type();
  TexGenTransition::init_type();
  CullFaceTransition::init_type();
  ColorMaskTransition::init_type();
  StencilTransition::init_type();
  TextureApplyTransition::init_type();
  ClipPlaneTransition::init_type();
  TransparencyTransition::init_type();
  FogTransition::init_type();
  LinesmoothTransition::init_type();
  PruneTransition::init_type();
  ColorTransition::init_type();
  BillboardTransition::init_type();
  DrawBoundsTransition::init_type();
  PointShapeTransition::init_type();
  PolygonOffsetTransition::init_type();
  ColorMatrixTransition::init_type();
  AlphaTransformTransition::init_type();

  // Register the RenderRelation class for the
  // NodeRelation::create_typed_arc() function.
  RenderRelation::register_with_factory();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  BillboardTransition::register_with_read_factory();
  ColorMatrixTransition::register_with_read_factory();
  ColorTransition::register_with_read_factory();
  CullFaceTransition::register_with_read_factory();
  DecalTransition::register_with_read_factory();
  DepthTestTransition::register_with_read_factory();
  DepthWriteTransition::register_with_read_factory();
  MaterialTransition::register_with_read_factory();
  PruneTransition::register_with_read_factory();
  RenderRelation::register_with_read_factory();
  TextureApplyTransition::register_with_read_factory();
  TextureTransition::register_with_read_factory();
  TransformTransition::register_with_read_factory();
  TransparencyTransition::register_with_read_factory();
}

