// Filename: config_sgattrib.cxx
// Created by:  drose (10Mar00)
// 
////////////////////////////////////////////////////////////////////

#include "config_sgattrib.h"
#include "renderRelation.h"
#include "colorTransition.h"
#include "colorAttribute.h"
#include "textureTransition.h"
#include "textureAttribute.h"
#include "decalTransition.h"
#include "decalAttribute.h"
#include "depthTestTransition.h"
#include "depthTestAttribute.h"
#include "depthWriteTransition.h"
#include "depthWriteAttribute.h"
#include "colorBlendTransition.h"
#include "colorBlendAttribute.h"
#include "renderModeTransition.h"
#include "renderModeAttribute.h"
#include "materialTransition.h"
#include "materialAttribute.h"
#include "texGenTransition.h"
#include "texGenAttribute.h"
#include "cullFaceTransition.h"
#include "cullFaceAttribute.h"
#include "colorMaskTransition.h"
#include "colorMaskAttribute.h"
#include "stencilTransition.h"
#include "stencilAttribute.h"
#include "textureApplyTransition.h"
#include "textureApplyAttribute.h"
#include "clipPlaneTransition.h"
#include "clipPlaneAttribute.h"
#include "transparencyTransition.h"
#include "transparencyAttribute.h"
#include "fogTransition.h"
#include "fogAttribute.h"
#include "linesmoothTransition.h"
#include "linesmoothAttribute.h"
#include "transformTransition.h"
#include "transformAttribute.h"
#include "texMatrixTransition.h"
#include "texMatrixAttribute.h"
#include "billboardTransition.h"
#include "showHideTransition.h"
#include "showHideAttribute.h"
#include "polygonOffsetTransition.h"
#include "polygonOffsetAttribute.h"
#include "pruneTransition.h"
#include "drawBoundsTransition.h"
#include "pointShapeTransition.h"
#include "pointShapeAttribute.h"
#include "colorMatrixAttribute.h"
#include "colorMatrixTransition.h"
#include "alphaTransformAttribute.h"
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
  TextureAttribute::init_type();
  TransformTransition::init_type();
  TransformAttribute::init_type();
  TexMatrixTransition::init_type();
  TexMatrixAttribute::init_type();
  DecalTransition::init_type();
  DecalAttribute::init_type();
  DepthTestTransition::init_type();
  DepthTestAttribute::init_type();
  DepthWriteTransition::init_type();
  DepthWriteAttribute::init_type();
  ColorBlendTransition::init_type();
  ColorBlendAttribute::init_type();
  RenderModeTransition::init_type();
  RenderModeAttribute::init_type();
  MaterialTransition::init_type();
  MaterialAttribute::init_type();
  TexGenTransition::init_type();
  TexGenAttribute::init_type();
  CullFaceTransition::init_type();
  CullFaceAttribute::init_type();
  ColorMaskTransition::init_type();
  ColorMaskAttribute::init_type();
  StencilTransition::init_type();
  StencilAttribute::init_type();
  TextureApplyTransition::init_type();
  TextureApplyAttribute::init_type();
  ClipPlaneTransition::init_type();
  ClipPlaneAttribute::init_type();
  TransparencyTransition::init_type();
  TransparencyAttribute::init_type();
  FogTransition::init_type();
  FogAttribute::init_type();
  LinesmoothTransition::init_type();
  LinesmoothAttribute::init_type();
  ShowHideTransition::init_type();
  ShowHideAttribute::init_type();
  PruneTransition::init_type();
  ColorTransition::init_type();
  ColorAttribute::init_type();
  BillboardTransition::init_type();
  DrawBoundsTransition::init_type();
  PointShapeTransition::init_type();
  PointShapeAttribute::init_type();
  PolygonOffsetTransition::init_type();
  PolygonOffsetAttribute::init_type();
  ColorMatrixTransition::init_type();
  ColorMatrixAttribute::init_type();
  AlphaTransformTransition::init_type();
  AlphaTransformAttribute::init_type();

  // Register the RenderRelation class for the
  // NodeRelation::create_typed_arc() function.
  RenderRelation::register_with_factory();

  //Registration of writeable object's creation
  //functions with BamReader's factory
  BillboardTransition::register_with_read_factory();
  ColorMatrixTransition::register_with_read_factory();
  CullFaceTransition::register_with_read_factory();
  DecalTransition::register_with_read_factory();
  DepthWriteTransition::register_with_read_factory();
  PruneTransition::register_with_read_factory();
  RenderRelation::register_with_read_factory();
  TextureApplyTransition::register_with_read_factory();
  TextureTransition::register_with_read_factory();
  TransformTransition::register_with_read_factory();
  TransparencyTransition::register_with_read_factory();
}

