/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file config_egg2pg.cxx
 * @author drose
 * @date 2002-02-26
 */

#include "config_egg2pg.h"

#include "dconfig.h"
#include "loaderFileTypeEgg.h"
#include "loaderFileTypeRegistry.h"
#include "configVariableManager.h"
#include "configVariableCore.h"
#include "eggRenderState.h"

#if !defined(CPPPARSER) && !defined(LINK_ALL_STATIC) && !defined(BUILDING_PANDA_EGG2PG)
  #error Buildsystem error: BUILDING_PANDA_EGG2PG not defined
#endif

ConfigureDef(config_egg2pg);
NotifyCategoryDef(egg2pg, "");

ConfigVariableDouble egg_normal_scale
("egg-normal-scale", 1.0);
ConfigVariableBool egg_show_normals
("egg-show-normals", false);

ConfigVariableEnum<CoordinateSystem> egg_coordinate_system
("egg-coordinate-system", CS_default);

ConfigVariableBool egg_ignore_mipmaps
("egg-ignore-mipmaps", false);
ConfigVariableBool egg_ignore_filters
("egg-ignore-filters", false);
ConfigVariableBool egg_ignore_decals
("egg-ignore-decals", false);
ConfigVariableBool egg_flatten
("egg-flatten", true,
 PRC_DESC("This is normally true to flatten out useless nodes after loading "
          "an egg file.  Set it false if you want to see the complete "
          "and true hierarchy as the egg loader created it (although the "
          "extra nodes may have a small impact on render performance)."));

ConfigVariableDouble egg_flatten_radius
("egg-flatten-radius", 0.0,
 PRC_DESC("This specifies the minimum cull radius in the egg file.  Nodes "
          "whose bounding volume is smaller than this radius will be "
          "flattened tighter than nodes larger than this radius, to "
          "reduce the node count even further.  The idea is that small "
          "objects will not need to have their individual components "
          "culled separately, but large environments should.  This allows "
          "the user to specify what should be considered \"small\".  Set "
          "it to 0.0 to disable this feature."));

ConfigVariableBool egg_unify
("egg-unify", true,
 PRC_DESC("When this is true, then in addition to flattening the scene graph "
          "nodes, the egg loader will also combine as many Geoms as "
          "possible within "
          "a given node into a single Geom.  This has theoretical performance "
          "benefits, especially on higher-end graphics cards, but it also "
          "slightly slows down egg loading."));

ConfigVariableBool egg_combine_geoms
("egg-combine-geoms", false,
 PRC_DESC("Set this true to combine sibling GeomNodes into a single GeomNode, "
          "when possible.  This usually shouldn't be necessary, since the "
          "egg loader does a pretty good job of combining these by itself."));

ConfigVariableBool egg_rigid_geometry
("egg-rigid-geometry", false,
 PRC_DESC("Set this true to create rigid pieces of an animated character as "
          "separate static nodes, or false to leave these in with the parent "
          "node as vertex-animated geometry.  Setting this true means less "
          "geometry has to be vertex-animated, but there will tend to be "
          "more separate pieces."));

ConfigVariableBool egg_flat_shading
("egg-flat-shading", false,
 PRC_DESC("Set this true to allow the egg loader to create geometry with the "
          "ShadeModelAttrib::M_flat attribute set.  It will do this only "
          "for geometry that has per-polygon normals and/or colors.  This "
          "allows the egg loader to avoid duplicating vertices when they "
          "are shared between connected polygons with different normals or "
          "colors, but it prevents the flat-shaded geometry from being "
          "combined with any adjacent smooth-shaded geometry (for instance, "
          "as the result of a flatten_strong operation).  It is false by "
          "default, since flat-shaded geometry is rare; but you may wish "
          "to set it true if your scene largely or entirely consists of "
          "flat-shaded polygons."));

ConfigVariableBool egg_flat_colors
("egg-flat-colors", true,
 PRC_DESC("Set this true to allow the egg loader to create geometry with the "
          "ColorAttrib::T_flat attribute set: that is, geometry that uses "
          "the scene graph color instead of per-vertex color.  Normally Panda "
          "will do this as an optimization for Geoms whose vertices are all "
          "the same color, or all white.  This allows the removal of the "
          "color attribute from the vertices where it is not necessary to "
          "specify colors per-vertex.  If this is false, the color attribute "
          "will always be specified per-vertex, even if all vertices have the "
          "same value."));

ConfigVariableBool egg_load_old_curves
("egg-load-old-curves", true,
 PRC_DESC("When this is true, a <NurbsCurve> entry appearing in an egg file "
          "will load as a NurbsCurve or ClassicNurbsCurve object (see below). "
          "When this is false, it will load a RopeNode instead, which uses the "
          "new NurbsCurveEvaluator interface."));


ConfigVariableBool egg_load_classic_nurbs_curves
("egg-load-classic-nurbs-curves", false,
 PRC_DESC("When this is true (and the above is also true), a <NurbsCurve> "
          "entry appearing in an egg file will load a ClassicNurbsCurve object "
          "instead of the default, a NurbsCurve object.  This only makes a "
          "difference when the NURBS++ library is available, in which case the "
          "default, NurbsCurve, is actually a NurbsPPCurve object."));


ConfigVariableBool egg_accept_errors
("egg-accept-errors", true,
 PRC_DESC("When this is true, certain kinds of recoverable errors (not syntax "
          "errors) in an egg file will be allowed and ignored when an egg file "
          "is loaded.  When it is false, only perfectly pristine egg files may "
          "be loaded."));


ConfigVariableBool egg_suppress_hidden
("egg-suppress-hidden", false,
 PRC_DESC("When this is true, objects flagged as \"hidden\" with the visibility "
          "scalar are not created at all.  When false, these objects are "
          "created, but initially stashed."));


ConfigVariableEnum<EggRenderMode::AlphaMode> egg_alpha_mode
("egg-alpha-mode", EggRenderMode::AM_blend,
 PRC_DESC("Specifies the alpha mode to apply when the alpha specification "
          "\"on\" appears in the egg file (or when a primitive is implicitly "
          "transparent, because of a <RGBA> that involves a non-unity alpha, "
          "or because of a four-channel texture."));

ConfigVariableInt egg_max_vertices
("egg-max-vertices", 65534,
 PRC_DESC("Specifies the maximum number of vertices that will be "
          "added to any one GeomVertexData by the egg loader."));

ConfigVariableInt egg_max_indices
("egg-max-indices", 65535,
 PRC_DESC("Specifies the maximum number of vertex indices that will be "
          "added to any one GeomPrimitive by the egg loader."));

ConfigVariableBool egg_emulate_bface
("egg-emulate-bface", true,
 PRC_DESC("When this is true, the bface flag applied to a polygon will "
          "cause two different polygons to be created, back-to-back.  When "
          "it is false, a single polygon will be created with the two_sided "
          "flag set on it."));

ConfigVariableBool egg_preload_simple_textures
("egg-preload-simple-textures", true,
 PRC_DESC("This specifies whether the egg loader will generate simple "
          "texture images for each texture loaded.  This supercedes the "
          "preload-simple-textures global default, for egg files.  In "
          "fact, the egg loader will generate simple texture images if "
          "either this or preload-simple-textures is true."));

ConfigVariableDouble egg_vertex_membership_quantize
("egg-vertex-membership-quantize", 0.1,
 PRC_DESC("Specifies the nearest amount to round each vertex joint "
          "membership value when loading an egg file.  This affects animated "
          "egg files only.  There is a substantial runtime "
          "performance advantage for reducing trivial differences in joint "
          "membership.  Set this to 0 to leave joint membership as it is."));

ConfigVariableInt egg_vertex_max_num_joints
("egg-vertex-max-num-joints", 4,
 PRC_DESC("Specifies the maximum number of distinct joints that are allowed "
          "to control any one vertex.  If a vertex requests assignment to "
          "more than this number of joints, the joints with the lesser membership "
          "value are ignored.  Set this to -1 to allow any number of joints."));

ConfigVariableBool egg_implicit_alpha_binary
("egg-implicit-alpha-binary", false,
 PRC_DESC("If this is true, then a <Scalar> alpha value appearing in an egg "
          "file that appears to specify only a binary (0 or 1) value for alpha "
          "will automatically be downgraded to alpha type \"binary\" instead of "
          "whatever appears in the egg file."));

ConfigureFn(config_egg2pg) {
  init_libegg2pg();
}

/**
 * Initializes the library.  This must be called at least once before any of
 * the functions or classes in this library can be used.  Normally it will be
 * called by the static initializers and need not be called explicitly, but
 * special cases exist.
 */
void
init_libegg2pg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  // Define a template for all egg-object-type-* variables, so the system
  // knows that these variables are defined when it finds them in a user's prc
  // file, even if we haven't actually read an egg file that uses the
  // particular <ObjectType> field.
  ConfigVariableManager *cv_mgr = ConfigVariableManager::get_global_ptr();
  cv_mgr->make_variable_template
    ("egg-object-type-*",
     ConfigVariableCore::VT_string, "",
     "Defines egg syntax for the named object type.",
     ConfigVariableCore::F_dynamic);

  EggRenderState::init_type();
  LoaderFileTypeEgg::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_global_ptr();

  reg->register_type(new LoaderFileTypeEgg);
}
