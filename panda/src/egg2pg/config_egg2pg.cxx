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
#include "get_config_path.h"
#include "loaderFileTypeEgg.h"
#include "loaderFileTypeRegistry.h"
#include "configVariableManager.h"
#include "configVariableCore.h"

ConfigureDef(config_egg2pg);
NotifyCategoryDef(egg2pg, "");

bool egg_mesh = config_egg2pg.GetBool("egg-mesh", true);
bool egg_retesselate_coplanar = config_egg2pg.GetBool("egg-retesselate-coplanar", true);
bool egg_unroll_fans = config_egg2pg.GetBool("egg-unroll-fans", true);
bool egg_show_tstrips = config_egg2pg.GetBool("egg-show-tstrips", false);
bool egg_show_qsheets = config_egg2pg.GetBool("egg-show-qsheets", false);
bool egg_show_quads = config_egg2pg.GetBool("egg-show-quads", false);
bool egg_false_color = (egg_show_tstrips | egg_show_qsheets | egg_show_quads);
bool egg_show_normals = config_egg2pg.GetBool("egg-show-normals", false);
double egg_normal_scale = config_egg2pg.GetDouble("egg-normal-scale", 1.0);
bool egg_subdivide_polys = config_egg2pg.GetBool("egg-subdivide-polys", true);
bool egg_consider_fans = config_egg2pg.GetBool("egg-consider-fans", true);
double egg_max_tfan_angle = config_egg2pg.GetDouble("egg-max-tfan-angle", 40.0);
int egg_min_tfan_tris = config_egg2pg.GetInt("egg-min-tfan-tris", 4);
double egg_coplanar_threshold = config_egg2pg.GetDouble("egg-coplanar-threshold", 0.01);
bool egg_ignore_mipmaps = config_egg2pg.GetBool("egg-ignore-mipmaps", false);
bool egg_ignore_filters = config_egg2pg.GetBool("egg-ignore-filters", false);
bool egg_ignore_clamp = config_egg2pg.GetBool("egg-ignore-clamp", false);
bool egg_ignore_decals = config_egg2pg.GetBool("egg-ignore-decals", false);
bool egg_flatten = config_egg2pg.GetBool("egg-flatten", true);

// Set this true to combine sibling GeomNodes into a single GeomNode,
// when possible.  This is probably a good idea in general, but we
// have it default to false for now for historical reasons (to avoid
// breaking code that assumes this doesn't happen).  Eventually the
// default will be set to true.
bool egg_combine_geoms = config_egg2pg.GetBool("egg-combine-geoms", false);

// Set this true to combine siblings of any combinable type into a
// single Node when possible.  It is almost always a bad idea to set
// this true.
bool egg_combine_siblings = config_egg2pg.GetBool("egg-combine-siblings", false);

bool egg_show_collision_solids = config_egg2pg.GetBool("egg-show-collision-solids", false);

// When this is true, a <NurbsCurve> entry appearing in an egg file
// will load as a NurbsCurve or ClassicNurbsCurve object (see below).
// When this is false, it will load a RopeNode instead, which uses the
// new NurbsCurveEvaluator interface.
bool egg_load_old_curves = config_egg2pg.GetBool("egg-load-old-curves", true);

// When this is true (and the above is also true), a <NurbsCurve>
// entry appearing in an egg file will load a ClassicNurbsCurve object
// instead of the default, a NurbsCurve object.  This only makes a
// difference when the NURBS++ library is available, in which case the
// default, NurbsCurve, is actually a NurbsPPCurve object.
bool egg_load_classic_nurbs_curves = config_egg2pg.GetBool("egg-load-classic-nurbs-curves", false);

// When this is true, certain kinds of recoverable errors (not syntax
// errors) in an egg file will be allowed and ignored when an egg file
// is loaded.  When it is false, only perfectly pristine egg files may
// be loaded.
bool egg_accept_errors = config_egg2pg.GetBool("egg-accept-errors", true);

// When this is true, objects flagged as "hidden" with the visibility
// scalar are not created at all.  When false, these objects are
// created, but initially stashed.
bool egg_suppress_hidden = config_egg2pg.GetBool("egg-suppress-hidden", false);


CoordinateSystem egg_coordinate_system = CS_invalid;
EggRenderMode::AlphaMode egg_alpha_mode = EggRenderMode::AM_unspecified;

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

  // Get egg-coordinate-system
  string csstr = config_egg2pg.GetString("egg-coordinate-system", "default");
  CoordinateSystem cs = parse_coordinate_system_string(csstr);

  if (cs == CS_invalid) {
    egg2pg_cat.error()
      << "Unexpected egg-coordinate-system string: " << csstr << "\n";
    cs = CS_default;
  }
  egg_coordinate_system = (cs == CS_default) ?
    default_coordinate_system : cs;

  // Get egg-alpha-mode
  string amstr = config_egg2pg.GetString("egg-alpha-mode", "blend");
  EggRenderMode::AlphaMode am = EggRenderMode::string_alpha_mode(amstr);

  if (am == EggRenderMode::AM_unspecified) {
    egg2pg_cat.error()
      << "Unexpected egg-alpha-mode string: " << amstr << "\n";
    egg_alpha_mode = EggRenderMode::AM_on;
  } else {
    egg_alpha_mode = am;
  }

  LoaderFileTypeEgg::init_type();

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();

  reg->register_type(new LoaderFileTypeEgg);
}
