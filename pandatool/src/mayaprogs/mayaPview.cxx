// Filename: mayaPview.cxx
// Created by:  drose (10Mar03)
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

#include "mayaPview.h"
#include "mayaToEggConverter.h"
#include "eggData.h"
#include "load_egg_file.h"
#include "config_util.h"
#include "textNode.h"
#include "multiplexStream.h"
#include "distanceUnit.h"
#include "configVariableEnum.h"

// We must define this to prevent Maya from doubly-declaring its
// MApiVersion string in this file as well as in libmayaegg.
#define _MApiVersion

#include "pre_maya_include.h"
#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MFileIO.h>
#include <maya/MArgParser.h>
#include <maya/MArgList.h>
#include <maya/MSyntax.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaPview::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
MayaPview::
MayaPview() {
}

////////////////////////////////////////////////////////////////////
//     Function: MayaPview::doIt
//       Access: Public, Virtual
//  Description: Called when the plugin command is invoked.
////////////////////////////////////////////////////////////////////
MStatus MayaPview::
doIt(const MArgList &args) {
  MStatus result;

  // First, parse the plugin arguments.
  MSyntax syntax;
  syntax.addFlag("a", "animate");

  MArgParser parser(syntax, args, &result);
  if (!result) {
    result.perror("arguments");
    return result;
  }

  bool animate = parser.isFlagSet("a", &result);
  if (!result) {
    result.perror("isFlagSet");
    return result;
  }

  // Maya seems to run each invocation of the plugin in a separate
  // thread.  To minimize conflict in our
  // not-yet-completely-thread-safe Panda, we'll create a separate
  // PandaFramework for each invocation, even though in principle we
  // could be sharing one framework for all of them.
  int argc = 0;
  char **argv = NULL;
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("Panda Viewer");
  framework.enable_default_keys();

  PT(WindowFramework) window;
  window = framework.open_window();
  if (window == (WindowFramework *)NULL) {
    // Couldn't open a window.
    nout << "Couldn't open a window!\n";
    return MS::kFailure;
  }

  // We've successfully opened a window.

  // Put up a "loading" message for the user's benefit.
  NodePath aspect_2d = window->get_aspect_2d();
  PT(TextNode) loading = new TextNode("loading");
  NodePath loading_np = aspect_2d.attach_new_node(loading);
  loading_np.set_scale(0.125f);
  loading->set_text_color(1.0f, 1.0f, 1.0f, 1.0f);
  loading->set_shadow_color(0.0f, 0.0f, 0.0f, 1.0f);
  loading->set_shadow(0.04f, 0.04f);
  loading->set_align(TextNode::A_center);
  loading->set_text("Loading...");

  // Allow a couple of frames to go by so the window will be fully
  // created and the text will be visible.
  framework.do_frame();
  framework.do_frame();

  window->enable_keyboard();
  window->setup_trackball();
  framework.get_models().instance_to(window->get_render());

  if (!convert(framework.get_models(), animate)) {
    nout << "failure in conversion.\n";
    return MS::kFailure;
  }

  nout << "successfully converted.\n";

  loading_np.remove_node();
  window->center_trackball(framework.get_models());
  window->loop_animations();

  if (animate) {
    window->set_anim_controls(true);
  }

  framework.main_loop();
  return MS::kSuccess;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaPview::creator
//       Access: Public, Static
//  Description: This is used to create a new instance of the plugin.
////////////////////////////////////////////////////////////////////
void *MayaPview::
creator() {
  return new MayaPview;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaPview::convert
//       Access: Private
//  Description: Actually converts the Maya selection to Panda
//               geometry, and parents it to the indicated NodePath.
////////////////////////////////////////////////////////////////////
bool MayaPview::
convert(const NodePath &parent, bool animate) {
  // Now make a converter to get all the Maya structures.
  MayaToEggConverter converter("plug-in");

  // We always want polygon output since we want to be able to see the
  // results.
  converter._polygon_output = true;

  if (animate) {
    // We also want to get the animation if there is any.
    converter.set_animation_convert(AC_both);
  }

  PathReplace *path_replace = converter.get_path_replace();

  // Accept relative pathnames in the Maya file.
  Filename source_file = 
    Filename::from_os_specific(MFileIO::currentFile().asChar());
  string source_dir = source_file.get_dirname();
  if (!source_dir.empty()) {
    path_replace->_path.append_directory(source_dir);
  }

  // Also search along the model path.
  path_replace->_path.append_path(get_model_path());

  PT(EggData) egg_data = new EggData;
  converter.set_egg_data(egg_data);
  converter.set_from_selection(true);

  if (!converter.convert_maya()) {
    nout << "Errors in conversion.\n";
    return false;
  }

  // Now the converter has filled up our egg structure with data, so
  // convert this egg data to Panda data for immediate viewing.
  DistanceUnit input_units = converter.get_input_units();
  ConfigVariableEnum<DistanceUnit> ptloader_units("ptloader-units", DU_invalid);
  if (input_units != DU_invalid && ptloader_units != DU_invalid && 
      input_units != ptloader_units) {
    // Convert the file to the units specified by the ptloader-units
    // Configrc variable.
    nout
      << "Converting from " << format_long_unit(input_units)
      << " to " << format_long_unit(ptloader_units) << "\n";
    double scale = convert_units(input_units, ptloader_units);
    egg_data->transform(LMatrix4d::scale_mat(scale));
  }

  egg_data->set_coordinate_system(CS_default);
  PT(PandaNode) result = load_egg_data(egg_data);

  if (result == (PandaNode *)NULL) {
    nout << "Unable to load converted egg data.\n";
    return false;
  }

  parent.attach_new_node(result);
  return true;
}




////////////////////////////////////////////////////////////////////
//     Function: initializePlugin
//  Description: Called by Maya when the plugin is loaded.
////////////////////////////////////////////////////////////////////
EXPCL_MISC MStatus 
initializePlugin(MObject obj) {
  // This code is just for debugging, to cause Notify to write its
  // output to a log file we can inspect, so we can see the error
  // messages output by DX7 or DX8 just before it does a panic exit
  // (and thereby shuts down Maya and its output window).
  /*
  MultiplexStream *local_nout = new MultiplexStream();
  Notify::ptr()->set_ostream_ptr(local_nout, 0);
  local_nout->add_file(Filename::expand_from("$TEMP/libmayapview.log"));
  local_nout->add_standard_output();
  */

  MFnPlugin plugin(obj, "VR Studio", "1.0");
  MStatus status;
  status = plugin.registerCommand("pview", MayaPview::creator);
  if (!status) {
    status.perror("registerCommand");
  }

  return status;
}

////////////////////////////////////////////////////////////////////
//     Function: uninitializePlugin
//  Description: Called by Maya when the plugin is unloaded.
////////////////////////////////////////////////////////////////////
EXPCL_MISC MStatus
uninitializePlugin(MObject obj) {
  MFnPlugin plugin(obj);
  MStatus status;
  status = plugin.deregisterCommand("pview");

  if (!status) {
    status.perror("deregisterCommand");
  }
  return status;
}
