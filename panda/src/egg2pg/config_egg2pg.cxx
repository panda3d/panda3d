// Filename: config_egg2pg.cxx
// Created by:  drose (26Feb02)
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

#include "config_egg2pg.h"

#include "dconfig.h"
#include "loaderFileTypeEgg.h"
#include "loaderFileTypeRegistry.h"
#include "configVariableManager.h"
#include "configVariableCore.h"
#include "eggRenderState.h"

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
          "nodes, the egg loader will also as many Geoms as possible within "
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

ConfigVariableBool egg_show_collision_solids
("egg-show-collision-solids", false);

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
("egg-alpha-mode", EggRenderMode::AM_blend);

ConfigureFn(config_egg2pg) {
  init_libegg2pg();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libegg2pg
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_libegg2pg() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

  // Define a template for all egg-object-type-* variables, so the
  // system knows that these variables are defined when it finds them
  // in a user's prc file, even if we haven't actually read an egg
  // file that uses the particular <ObjectType> field.
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
