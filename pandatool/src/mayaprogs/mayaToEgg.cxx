// Filename: mayaToEgg.cxx
// Created by:  drose (15Feb00)
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

#include "mayaToEgg.h"
#include "mayaToEggConverter.h"
#include "config_mayaegg.h"
#include "config_maya.h"  // for maya_cat
#include "globPattern.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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
     &MayaToEgg::dispatch_double, NULL, &_polygon_tolerance);

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
    ("trans", "type", 0,
     "Specifies which transforms in the Maya file should be converted to "
     "transforms in the egg file.  The option may be one of all, model, "
     "dcs, or none.  The default is model, which means only transforms on "
     "nodes that have the model flag or the dcs flag are preserved.",
     &MayaToEgg::dispatch_transform_type, NULL, &_transform_type);

  add_option
    ("subset", "name", 0,
     "Specifies that only a subset of the geometry in the Maya file should "
     "be converted; specifically, the geometry under the node or nodes whose "
     "name matches the parameter (which may include globbing characters "
     "like * or ?).  This parameter may be repeated multiple times to name "
     "multiple roots.  If it is omitted altogether, the entire file is "
     "converted.",
     &MayaToEgg::dispatch_vector_string, NULL, &_subsets);

  add_option
    ("ignore-slider", "name", 0,
     "Specifies the name of a slider (blend shape deformer) that maya2egg "
     "should not process.  The slider will not be touched during conversion "
     "and it will not become a part of the animation.  This "
     "parameter may including globbing characters, and it may be repeated "
     "as needed.",
     &MayaToEgg::dispatch_vector_string, NULL, &_ignore_sliders);

  add_option
    ("force-joint", "name", 0,
     "Specifies the name of a DAG node that maya2egg "
     "should treat as a joint, even if it does not appear to be a Maya joint "
     "and does not appear to be animated.",
     &MayaToEgg::dispatch_vector_string, NULL, &_force_joints);

  add_option
    ("v", "", 0,
     "Increase verbosity.  More v's means more verbose.",
     &MayaToEgg::dispatch_count, NULL, &_verbose);

  // Unfortunately, the Maya API doesn't allow us to differentiate
  // between relative and absolute pathnames--everything comes out as
  // an absolute pathname, even if it is stored in the Maya file as a
  // relative path.  So we can't support -noabs.
  remove_option("noabs");

  _verbose = 0;
  _polygon_tolerance = 0.01;
  _transform_type = MayaToEggConverter::TT_model;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
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

  // Let's convert the output file to a full path before we initialize
  // Maya, since Maya now has a nasty habit of changing the current
  // directory.
  if (_got_output_filename) {
    _output_filename.make_absolute();
  }

  nout << "Initializing Maya.\n";
  MayaToEggConverter converter(_program_name);
  if (!converter.open_api()) {
    nout << "Unable to initialize Maya.\n";
    exit(1);
  }

  // Copy in the command-line parameters.
  converter._polygon_output = _polygon_output;
  converter._polygon_tolerance = _polygon_tolerance;
  converter._respect_maya_double_sided = _respect_maya_double_sided;
  converter._always_show_vertex_color = !_suppress_vertex_color;
  converter._transform_type = _transform_type;

  vector_string::const_iterator si;
  if (!_subsets.empty()) {
    converter.clear_subsets();
    for (si = _subsets.begin(); si != _subsets.end(); ++si) {
      converter.add_subset(GlobPattern(*si));
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

  // Use the standard Maya units, if the user didn't specify
  // otherwise.  This always returns centimeters, which is the way all
  // Maya files are stored internally (and is the units returned by
  // all of the API functions called here).
  if (_input_units == DU_invalid) {
    _input_units = converter.get_input_units();
  }

  write_egg_file();
  nout << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEgg::dispatch_transform_type
//       Access: Protected, Static
//  Description: Dispatches a parameter that expects a
//               MayaToEggConverter::TransformType option.
////////////////////////////////////////////////////////////////////
bool MayaToEgg::
dispatch_transform_type(const string &opt, const string &arg, void *var) {
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
