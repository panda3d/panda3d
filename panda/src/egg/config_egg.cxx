/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_egg.cxx
 * @author drose
 * @date 2000-03-19
 */

#include "config_egg.h"
#include "eggRenderMode.h"
#include "eggAnimData.h"
#include "eggAnimPreload.h"
#include "eggAttributes.h"
#include "eggBin.h"
#include "eggBinMaker.h"
#include "eggComment.h"
#include "eggCompositePrimitive.h"
#include "eggCoordinateSystem.h"
#include "eggCurve.h"
#include "eggExternalReference.h"
#include "eggFilenameNode.h"
#include "eggGroup.h"
#include "eggGroupNode.h"
#include "eggGroupUniquifier.h"
#include "eggLine.h"
#include "eggMaterial.h"
#include "eggNameUniquifier.h"
#include "eggNamedObject.h"
#include "eggNode.h"
#include "eggNurbsCurve.h"
#include "eggNurbsSurface.h"
#include "eggObject.h"
#include "eggPatch.h"
#include "eggPoint.h"
#include "eggPolygon.h"
#include "eggPolysetMaker.h"
#include "eggPoolUniquifier.h"
#include "eggPrimitive.h"
#include "eggSAnimData.h"
#include "eggSurface.h"
#include "eggSwitchCondition.h"
#include "eggTable.h"
#include "eggTexture.h"
#include "eggTriangleFan.h"
#include "eggTriangleStrip.h"
#include "eggUserData.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggVertexUV.h"
#include "eggVertexAux.h"
#include "eggXfmAnimData.h"
#include "eggXfmSAnim.h"

#include "dconfig.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_EGG)
  #error Buildsystem error: BUILDING_PANDA_EGG not defined
#endif

Configure(config_egg);
NotifyCategoryDef(egg, "");

ConfigureFn(config_egg) {
  init_libegg();
}

ConfigVariableBool egg_support_old_anims
("egg-support-old-anims", true,
 PRC_DESC("Set this true to support loading of old character animation files, which "
          "had the convention that the order \"phr\" implied a reversed roll."));

ConfigVariableBool egg_mesh
("egg-mesh", true,
 PRC_DESC("Set this true to convert triangles and higher-order polygons "
          "into triangle strips and triangle fans when an egg file is "
          "loaded or converted to bam.  Set this false just to triangulate "
          "everything into independent triangles."));

ConfigVariableBool egg_retesselate_coplanar
("egg-retesselate-coplanar", false,
 PRC_DESC("If this is true, the egg loader may reverse the "
          "tesselation direction of a single pair of planar triangles that "
          "share the same properties, if that will help get a better "
          "triangle strip.  In some rare cases, doing so can distort the "
          "UV's on a face; turning this off should eliminate that artifact "
          "(at the cost of less-effective triangle stripping)."));

ConfigVariableBool egg_unroll_fans
("egg-unroll-fans", true,
 PRC_DESC("Set this true to allow the egg loader to convert weak triangle "
          "fans--triangles that share the same vertex but aren't "
          "connected enough to justify making a triangle fan primitive "
          "from them--into a series of zig-zag triangles that can make "
          "a triangle strip that might connect better with its neighbors."));

ConfigVariableBool egg_show_tstrips
("egg-show-tstrips", false,
 PRC_DESC("Set this true to color each triangle strip a random color, with "
          "the leading triangle a little bit darker, so you can visually "
          "observe the quality of the triangle stripping algorithm."));

ConfigVariableBool egg_show_qsheets
("egg-show-qsheets", false,
 PRC_DESC("Set this true to color each quadsheet a random color, so you "
          "can visually observe the quadsheet algorithm."));

ConfigVariableBool egg_show_quads
("egg-show-quads", false,
 PRC_DESC("Set this true to color each detected quad a random color, so "
          "you can visually observe the algorithm that unifies pairs of "
          "triangles into quads (prior to generating triangle strips)."));

ConfigVariableBool egg_subdivide_polys
("egg-subdivide-polys", true,
 PRC_DESC("This is obsolete.  In the old Geom implementation, it used to "
          "be true to force higher-order polygons that were not otherwise "
          "meshed to be subdivided into triangles.  In the new "
          "Geom implementation, this happens anyway."));

ConfigVariableBool egg_consider_fans
("egg-consider-fans", false,
 PRC_DESC("Set this true to enable the egg mesher to consider making "
          "triangle fans out of triangles that are connected at a common "
          "vertex.  This may help if your scene involves lots of such "
          "connected triangles, but it can also make the overall stripping "
          "less effective (by interfering with triangle strips)."));

ConfigVariableDouble egg_max_tfan_angle
("egg-max-tfan-angle", 40.0,
 PRC_DESC("The maximum average angle per triangle to allow in a triangle "
          "fan.  If triangles are larger than this--that is, more loosely "
          "packed--then we figure a triangle strip is likely to do a "
          "more effective job than a triangle fan, and the fan maker leaves "
          "it alone."));

ConfigVariableInt egg_min_tfan_tris
("egg-min-tfan-tris", 4,
 PRC_DESC("The minimum number of triangles that must be involved in order "
          "to generate a triangle fan.  Fewer than this is just interrupting "
          "a triangle strip."));

ConfigVariableDouble egg_coplanar_threshold
("egg-coplanar-threshold", 0.01,
 PRC_DESC("The numerical threshold below which polygons are considered "
          "to be coplanar.  Determined empirically."));

ConfigVariableInt egg_test_vref_integrity
("egg-test-vref-integrity", 20,
 PRC_DESC("The maximum number of vertices a primitive may have before "
          "its vertices will no longer be checked for internal integrity.  "
          "This is meaningful in non-production builds only."));

ConfigVariableInt egg_recursion_limit
("egg-recursion-limit", 1000,
 PRC_DESC("The maximum number of levels that recursive algorithms within "
          "the egg library are allowed to traverse.  This is a simple hack "
          "to prevent deeply-recursive algorithms from triggering a stack "
          "overflow.  Set it larger to run more efficiently if your stack "
          "allows it; set it lower if you experience stack overflows."));

ConfigVariableInt egg_precision
("egg-precision", 15,
 PRC_DESC("The number of digits of precision to write out for values in "
          "an egg file.  Leave this at 0 to use the default setting for the "
          "stream."));

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libegg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  EggRenderMode::init_type();
  EggAnimData::init_type();
  EggAnimPreload::init_type();
  EggAttributes::init_type();
  EggBin::init_type();
  EggBinMaker::init_type();
  EggComment::init_type();
  EggCompositePrimitive::init_type();
  EggCoordinateSystem::init_type();
  EggCurve::init_type();
  EggData::init_type();
  EggExternalReference::init_type();
  EggFilenameNode::init_type();
  EggGroup::init_type();
  EggGroupNode::init_type();
  EggGroupUniquifier::init_type();
  EggLine::init_type();
  EggMaterial::init_type();
  EggNameUniquifier::init_type();
  EggNamedObject::init_type();
  EggNode::init_type();
  EggNurbsCurve::init_type();
  EggNurbsSurface::init_type();
  EggObject::init_type();
  EggPatch::init_type();
  EggPoint::init_type();
  EggPolygon::init_type();
  EggPolysetMaker::init_type();
  EggPoolUniquifier::init_type();
  EggPrimitive::init_type();
  EggSAnimData::init_type();
  EggSurface::init_type();
  EggSwitchCondition::init_type();
  EggSwitchConditionDistance::init_type();
  EggTable::init_type();
  EggTexture::init_type();
  EggTriangleFan::init_type();
  EggTriangleStrip::init_type();
  EggUserData::init_type();
  EggVertex::init_type();
  EggVertexPool::init_type();
  EggVertexUV::init_type();
  EggVertexAux::init_type();
  EggXfmAnimData::init_type();
  EggXfmSAnim::init_type();
}
