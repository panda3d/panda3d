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

ConfigureDef(config_egg2pg);
NotifyCategoryDef(egg2pg, "");

ConfigVariableBool egg_mesh
("egg-mesh", true);
ConfigVariableBool egg_retesselate_coplanar
("egg-retesselate-coplanar", true);
ConfigVariableBool egg_unroll_fans
("egg-unroll-fans", true);
ConfigVariableBool egg_show_tstrips
("egg-show-tstrips", false);
ConfigVariableBool egg_show_qsheets
("egg-show-qsheets", false);
ConfigVariableBool egg_show_quads
("egg-show-quads", false);
ConfigVariableBool egg_show_normals
("egg-show-normals", false);
ConfigVariableDouble egg_normal_scale
("egg-normal-scale", 1.0);
ConfigVariableBool egg_subdivide_polys
("egg-subdivide-polys", true);
ConfigVariableBool egg_consider_fans
("egg-consider-fans", true);
ConfigVariableDouble egg_max_tfan_angle
("egg-max-tfan-angle", 40.0);
ConfigVariableInt egg_min_tfan_tris
("egg-min-tfan-tris", 4);
ConfigVariableDouble egg_coplanar_threshold
("egg-coplanar-threshold", 0.01);

ConfigVariableEnum<CoordinateSystem> egg_coordinate_system
("egg-coordinate-system", CS_default);

ConfigVariableBool egg_ignore_mipmaps
("egg-ignore-mipmaps", false);
ConfigVariableBool egg_ignore_filters
("egg-ignore-filters", false);
ConfigVariableBool egg_ignore_clamp
("egg-ignore-clamp", false);
ConfigVariableBool egg_ignore_decals
("egg-ignore-decals", false);
ConfigVariableBool egg_flatten
("egg-flatten", true);

ConfigVariableBool egg_combine_geoms
("egg-combine-geoms", false,
 PRC_DESC("Set this true to combine sibling GeomNodes into a single GeomNode, "
          "when possible.  This is probably a good idea in general, but we "
          "have it default to false for now for historical reasons (to avoid "
          "breaking code that assumes this doesn't happen).  Eventually the "
          "default may be set to true."));


ConfigVariableBool egg_combine_siblings
("egg-combine-siblings", false,
 PRC_DESC("Set this true to combine siblings of any combinable type into a "
          "single Node when possible.  It is almost always a bad idea to set "
          "this true."));


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

  LoaderFileTypeEgg::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();

  reg->register_type(new LoaderFileTypeEgg);
}
