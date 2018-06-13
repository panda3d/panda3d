/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEgg.cxx
 * @author drose
 * @date 2000-02-15
 *
 * Additional Maintenance by the PandaSE team
 * Carnegie Mellon Entertainment Technology Center
 * Spring '10
 * Team Members:
 * Deepak Chandraskeran - producer / programmer
 * Andrew Gartner - programmer/technical artist
 * Federico Perazzi - programmer
 * Shuying Feng - programmer
 * Wei-Feng Huang - programmer
 * (Egger additions by Andrew Gartner and Wei-Feng Huang)
 * The egger can now support vertex color in a variety
 * of combinations with flat color and file color textures
 * (see set_vertex_color).  Also, there are two new
 * command line options "legacy-shaders" and "texture-copy".
 * The first treats any Maya material/shader as if it were
 * a legacy shader. Passing it through the legacy codepath.
 * This feature was originally intended to fix a bug where
 * flat-color was being ignored in the modern (Phong) codepath
 * However, with the new vertex and flat color functions it
 * may not be necessary.  Still, until the newer color functions
 * have been tried and tested more, the feature has been left in
 * to anticipate any problems that may arise. The texture copy
 * feature was added to provide a way to resolve build path issues
 * and can support both relative and absolute paths. The feature
 * will copy any file maps/textures to the specified directory
 * and update the egg file accordingly.
 */

#include "mayaToEgg.h"
#include "mayaToEggConverter.h"
#include "config_mayaegg.h"
#include "config_maya.h"  // for maya_cat
#include "globPattern.h"

/**
 *
 */
MayaToEgg::
MayaToEgg() :
  SomethingToEgg("Maya", ".mb")
{
  add_path_replace_options();
  add_path_store_options();
  add_animation_options();
  add_units_options();
  add_normals_options();
  add_transform_options();

  set_program_brief("convert Maya model files to .egg");
  set_program_description
    ("This program converts Maya model files to egg.  Static and animatable "
     "models can be converted, with polygon or NURBS output.  Animation tables "
     "can also be generated to apply to an animatable model.");

  add_option
    ("p", "", 0,
     "Generate polygon output only.  Tesselate all NURBS surfaces to "
     "polygons via the built-in Maya tesselator.  The tesselation will "
     "be based on the tolerance factor given by -ptol.",
     &MayaToEgg::dispatch_none, &_polygon_output);

  add_option
    ("ptol", "tolerance", 0,
     "Specify the fit tolerance for Maya polygon tesselation.  The smaller "
     "the number, the more polygons will be generated.  The default is "
     "0.01.",
     &MayaToEgg::dispatch_double, nullptr, &_polygon_tolerance);

  add_option
    ("bface", "", 0,
     "Respect the Maya \"double sided\" rendering flag to indicate whether "
     "polygons should be double-sided or single-sided.  Since this flag "
     "is set to double-sided by default in Maya, it is often better to "
     "ignore this flag (unless your modelers are diligent in turning it "
     "off where it is not desired).  If this flag is not specified, the "
     "default is to treat all polygons as single-sided, unless an "
     "egg object type of \"double-sided\" is set.",
     &MayaToEgg::dispatch_none, &_respect_maya_double_sided);

  add_option
    ("suppress-vcolor", "", 0,
     "Ignore vertex color for geometry that has a texture applied.  "
     "(This is the way Maya normally renders internally.)  The egg flag "
     "'vertex-color' may be applied to a particular model to override "
     "this setting locally.",
     &MayaToEgg::dispatch_none, &_suppress_vertex_color);

  add_option
    ("convert-cameras", "", 0,
     "Convert all camera nodes to locators. Will preserve position and rotation.",
     &MayaToEgg::dispatch_none, &_convert_cameras);

  add_option
    ("convert-lights", "", 0,
     "Convert all light nodes to locators. Will preserve position and rotation only.",
     &MayaToEgg::dispatch_none, &_convert_lights);

  add_option
    ("keep-uvs", "", 0,
     "Convert all UV sets on all vertices, even those that do not appear "
     "to be referenced by any textures.",
     &MayaToEgg::dispatch_none, &_keep_all_uvsets);

  add_option
    ("round-uvs", "", 0,
     "round up uv coordinates to the nearest 1/100th. i.e. -0.001 becomes"
     "0.0; 0.444 becomes 0.44; 0.778 becomes 0.78.",
     &MayaToEgg::dispatch_none, &_round_uvs);

  add_option
    ("copytex", "dir", 41,
     "Legacy option.  Same as -pc.",
     &MayaToEgg::dispatch_filename, &_legacy_copytex, &_legacy_copytex_dir);

  add_option
    ("trans", "type", 0,
     "Specifies which transforms in the Maya file should be converted to "
     "transforms in the egg file.  The option may be one of all, model, "
     "dcs, or none.  The default is model, which means only transforms on "
     "nodes that have the model flag or the dcs flag are preserved.",
     &MayaToEgg::dispatch_transform_type, nullptr, &_transform_type);

  add_option
    ("subroot", "name", 0,
     "Specifies that only a subroot of the geometry in the Maya file should "
     "be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.  If it is omitted altogether, the entire file is "
     "converted.",
     &MayaToEgg::dispatch_vector_string, nullptr, &_subroots);

  add_option
    ("subset", "name", 0,
     "Specifies that only a subset of the geometry in the Maya file should "
     "be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.  If it is omitted altogether, the entire file is "
     "converted.",
     &MayaToEgg::dispatch_vector_string, nullptr, &_subsets);

  add_option
    ("exclude", "name", 0,
     "Specifies that a subset of the geometry in the Maya file should "
     "not be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.",
     &MayaToEgg::dispatch_vector_string, nullptr, &_excludes);

  add_option
    ("ignore-slider", "name", 0,
     "Specifies the name of a slider (blend shape deformer) that maya2egg "
     "should not process.  The slider will not be touched during conversion "
     "and it will not become a part of the animation.  This "
     "parameter may including globbing characters, and it may be repeated "
     "as needed.",
     &MayaToEgg::dispatch_vector_string, nullptr, &_ignore_sliders);

  add_option
    ("force-joint", "name", 0,
     "Specifies the name of a DAG node that maya2egg "
     "should treat as a joint, even if it does not appear to be a Maya joint "
     "and does not appear to be animated.",
     &MayaToEgg::dispatch_vector_string, nullptr, &_force_joints);

  add_option
    ("v", "", 0,
     "Increase verbosity.  More v's means more verbose.",
     &MayaToEgg::dispatch_count, nullptr, &_verbose);

  add_option
    ("legacy-shaders", "", 0,
     "Use this flag to turn off modern (Phong) shader generation"
     "and treat all shaders as if they were Lamberts (legacy).",
     &MayaToEgg::dispatch_none, &_legacy_shader);

  // Unfortunately, the Maya API doesn't allow us to differentiate between
  // relative and absolute pathnames--everything comes out as an absolute
  // pathname, even if it is stored in the Maya file as a relative path.  So
  // we can't support -noabs.
  remove_option("noabs");

  _verbose = 0;
  _polygon_tolerance = 0.01;
  _transform_type = MayaToEggConverter::TT_model;
  _got_tbnauto = true;
}

/**
 *
 */
void MayaToEgg::
run() {
  // Set the verbose level by using Notify.
  if (_verbose >= 3) {
    maya_cat->set_severity(NS_spam);
    mayaegg_cat->set_severity(NS_spam);
  } else if (_verbose >= 2) {
    maya_cat->set_severity(NS_debug);
    mayaegg_cat->set_severity(NS_debug);
  } else if (_verbose >= 1) {
    maya_cat->set_severity(NS_info);
    mayaegg_cat->set_severity(NS_info);
  }

  if (_legacy_copytex && !_path_replace->_copy_files) {
    _path_replace->_copy_files = true;
    _path_replace->_copy_into_directory = _legacy_copytex_dir;
  }

  // Let's convert the output file to a full path before we initialize Maya,
  // since Maya now has a nasty habit of changing the current directory.
  if (_got_output_filename) {
    _output_filename.make_absolute();
    _path_replace->_path_directory.make_absolute();
  }

  nout << "Initializing Maya.\n";
  MayaToEggConverter converter(_program_name);
  // reverting directories is really not needed for maya2egg.  It's more
  // needed for mayaeggloader and such
  if (!converter.open_api(false)) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  // Copy in the command-line parameters.
  converter._polygon_output = _polygon_output;
  converter._polygon_tolerance = _polygon_tolerance;
  converter._respect_maya_double_sided = _respect_maya_double_sided;
  converter._always_show_vertex_color = !_suppress_vertex_color;
  converter._keep_all_uvsets = _keep_all_uvsets;
  converter._convert_cameras = _convert_cameras;
  converter._convert_lights = _convert_lights;
  converter._round_uvs = _round_uvs;
  converter._transform_type = _transform_type;
  converter._legacy_shader = _legacy_shader;

  vector_string::const_iterator si;
  if (!_subroots.empty()) {
    converter.clear_subroots();
    for (si = _subroots.begin(); si != _subroots.end(); ++si) {
      converter.add_subroot(GlobPattern(*si));
    }
  }

  if (!_subsets.empty()) {
    converter.clear_subsets();
    for (si = _subsets.begin(); si != _subsets.end(); ++si) {
      converter.add_subset(GlobPattern(*si));
    }
  }

  if (!_excludes.empty()) {
    converter.clear_excludes();
    for (si = _excludes.begin(); si != _excludes.end(); ++si) {
      converter.add_exclude(GlobPattern(*si));
    }
  }

  if (!_ignore_sliders.empty()) {
    converter.clear_ignore_sliders();
    for (si = _ignore_sliders.begin(); si != _ignore_sliders.end(); ++si) {
      converter.add_ignore_slider(GlobPattern(*si));
    }
  }

  if (!_force_joints.empty()) {
    converter.clear_force_joints();
    for (si = _force_joints.begin(); si != _force_joints.end(); ++si) {
      converter.add_force_joint(GlobPattern(*si));
    }
  }

  // Copy in the path and animation parameters.
  apply_parameters(converter);

  // Set the coordinate system to match Maya's.
  if (!_got_coordinate_system) {
    _coordinate_system = converter._maya->get_coordinate_system();
  }
  _data->set_coordinate_system(_coordinate_system);

  converter.set_egg_data(_data);

  if (!converter.convert_file(_input_filename)) {
    nout << "Errors in conversion.\n";
    exit(1);
  }

  // Use the standard Maya units, if the user didn't specify otherwise.  This
  // always returns centimeters, which is the way all Maya files are stored
  // internally (and is the units returned by all of the API functions called
  // here).
  if (_input_units == DU_invalid) {
    _input_units = converter.get_input_units();
  }

  write_egg_file();
  nout << "\n";
}

/**
 * Dispatches a parameter that expects a MayaToEggConverter::TransformType
 * option.
 */
bool MayaToEgg::
dispatch_transform_type(const std::string &opt, const std::string &arg, void *var) {
  MayaToEggConverter::TransformType *ip = (MayaToEggConverter::TransformType *)var;
  (*ip) = MayaToEggConverter::string_transform_type(arg);

  if ((*ip) == MayaToEggConverter::TT_invalid) {
    nout << "Invalid type for -" << opt << ": " << arg << "\n"
         << "Valid types are all, model, dcs, and none.\n";
    return false;
  }

  return true;
}

int main(int argc, char *argv[]) {
  MayaToEgg prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
