// Filename: config_egg2sg.cxx
// Created by:  drose (01Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "config_egg2sg.h"
#include "loaderFileTypeEgg.h"

#include <dconfig.h>
#include <loaderFileTypeRegistry.h>
#include <get_config_path.h>

ConfigureDef(config_egg2sg);
NotifyCategoryDef(egg2sg, "");

const bool egg_mesh = config_egg2sg.GetBool("egg-mesh", true);
const bool egg_retesselate_coplanar = config_egg2sg.GetBool("egg-retesselate-coplanar", true);
const bool egg_unroll_fans = config_egg2sg.GetBool("egg-unroll-fans", true);
const bool egg_show_tstrips = config_egg2sg.GetBool("egg-show-tstrips", false);
const bool egg_show_qsheets = config_egg2sg.GetBool("egg-show-qsheets", false);
const bool egg_show_quads = config_egg2sg.GetBool("egg-show-quads", false);
const bool egg_false_color = (egg_show_tstrips | egg_show_qsheets | egg_show_quads);
const bool egg_show_normals = config_egg2sg.GetBool("egg-show-normals", false);
const double egg_normal_scale = config_egg2sg.GetDouble("egg-normal-scale", 1.0);
const bool egg_subdivide_polys = config_egg2sg.GetBool("egg-subdivide-polys", true);
const bool egg_consider_fans = config_egg2sg.GetBool("egg-consider-fans", true);
const double egg_max_tfan_angle = config_egg2sg.GetDouble("egg-max-tfan-angle", 40.0);
const int egg_min_tfan_tris = config_egg2sg.GetInt("egg-min-tfan-tris", 4);
const double egg_coplanar_threshold = config_egg2sg.GetDouble("egg-coplanar-threshold", 0.01);
const bool egg_ignore_mipmaps = config_egg2sg.GetBool("egg-ignore-mipmaps", false);
const bool egg_ignore_filters = config_egg2sg.GetBool("egg-ignore-filters", false);
const bool egg_ignore_clamp = config_egg2sg.GetBool("egg-ignore-clamp", false);
const bool egg_always_decal_textures = config_egg2sg.GetBool("egg-always-decal-textures", false);
const bool egg_ignore_decals = config_egg2sg.GetBool("egg-ignore-decals", false);
const bool egg_flatten = config_egg2sg.GetBool("egg-flatten", true);

// It is almost always a bad idea to set this true.
const bool egg_flatten_siblings = config_egg2sg.GetBool("egg-flatten-siblings", false);

const bool egg_show_collision_solids = config_egg2sg.GetBool("egg-show-collision-solids", false);

CoordinateSystem egg_coordinate_system;

ConfigureFn(config_egg2sg) {
  LoaderFileTypeEgg::init_type();

  string csstr = config_egg2sg.GetString("egg-coordinate-system", "default");
  CoordinateSystem cs = parse_coordinate_system_string(csstr);

  if (cs == CS_invalid) {
    egg2sg_cat.error()
      << "Unexpected egg-coordinate-system string: " << csstr << "\n";
    cs = CS_default;
  }
  egg_coordinate_system = (cs == CS_default) ? 
    default_coordinate_system : cs;

  LoaderFileTypeRegistry *reg = LoaderFileTypeRegistry::get_ptr();

  reg->register_type(new LoaderFileTypeEgg);
}
