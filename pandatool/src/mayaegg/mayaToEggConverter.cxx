// Filename: mayaToEggConverter.cxx
// Created by:  drose (10Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "mayaToEggConverter.h"
#include "mayaShader.h"
#include "maya_funcs.h"
#include "config_mayaegg.h"
#include "mayaEggGroupUserData.h"

#include "eggData.h"
#include "eggGroup.h"
#include "eggTable.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggNurbsSurface.h"
#include "eggNurbsCurve.h"
#include "eggPolygon.h"
#include "eggPrimitive.h"
#include "eggTexture.h"
#include "eggTextureCollection.h"
#include "eggXfmSAnim.h"
#include "string_utils.h"
#include "dcast.h"

#include "pre_maya_include.h"
#include <maya/MArgList.h>
#include <maya/MColor.h>
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnTransform.h>
#include <maya/MFnLight.h>
#include <maya/MFnNurbsSurface.h>
#include <maya/MFnNurbsCurve.h>
#include <maya/MFnMesh.h>
#include <maya/MFnMeshData.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MFnPlugin.h>
#include <maya/MItDag.h>
#include <maya/MLibrary.h>
#include <maya/MMatrix.h>
#include <maya/MObject.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MIntArray.h>
#include <maya/MPxCommand.h>
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MVector.h>
#include <maya/MTesselationParams.h>
#include <maya/MAnimControl.h>
#include <maya/MGlobal.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnSkinCluster.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPathArray.h>
#include <maya/MSelectionList.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
MayaToEggConverter(const string &program_name) :
  _program_name(program_name)
{
  _from_selection = false;
  _polygon_output = false;
  _polygon_tolerance = 0.01;
  _respect_maya_double_sided = maya_default_double_sided;
  _always_show_vertex_color = maya_default_vertex_color;
  _transform_type = TT_model;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
MayaToEggConverter(const MayaToEggConverter &copy) :
  _from_selection(copy._from_selection),
  _maya(copy._maya),
  _polygon_output(copy._polygon_output),
  _polygon_tolerance(copy._polygon_tolerance),
  _respect_maya_double_sided(copy._respect_maya_double_sided),
  _always_show_vertex_color(copy._always_show_vertex_color),
  _transform_type(copy._transform_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
~MayaToEggConverter() {
  close_api();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *MayaToEggConverter::
make_copy() {
  return new MayaToEggConverter(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string MayaToEggConverter::
get_name() const {
  return "Maya";
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string MayaToEggConverter::
get_extension() const {
  return "mb";
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
//
//               This is designed to be as generic as possible,
//               generally in support of run-time loading.
//               Also see convert_maya().
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_file(const Filename &filename) {
  if (!open_api()) {
    mayaegg_cat.error()
      << "Maya is not available.\n";
    return false;
  }
  if (!_maya->read(filename)) {
    mayaegg_cat.error()
      << "Unable to read " << filename << "\n";
    return false;
  }

  if (_character_name.empty()) {
    _character_name = filename.get_basename_wo_extension();
  }

  return convert_maya(false);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_maya
//       Access: Public
//  Description: Fills up the egg_data structure according to the
//               global maya model data.  Returns true if successful,
//               false if there is an error.  If from_selection is
//               true, the converted geometry is based on that which
//               is selected; otherwise, it is the entire Maya scene.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_maya(bool from_selection) {
  _from_selection = from_selection;
  _textures.clear();
  _shaders.clear();

  if (!open_api()) {
    mayaegg_cat.error()
      << "Maya is not available.\n";
    return false;
  }

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(_maya->get_coordinate_system());
  }

  mayaegg_cat.info()
    << "Converting from Maya.\n";

  // Figure out the animation parameters.
  double start_frame, end_frame, frame_inc, input_frame_rate, output_frame_rate;
  if (has_start_frame()) {
    start_frame = get_start_frame();
  } else {
    start_frame = MAnimControl::minTime().value();
  }
  if (has_end_frame()) {
    end_frame = get_end_frame();
  } else {
    end_frame = MAnimControl::maxTime().value();
  }
  if (has_frame_inc()) {
    frame_inc = get_frame_inc();
  } else {
    frame_inc = 1.0;
  }
  if (has_input_frame_rate()) {
    input_frame_rate = get_input_frame_rate();
  } else {
    MTime time(1.0, MTime::kSeconds);
    input_frame_rate = time.as(MTime::uiUnit());
  }
  if (has_output_frame_rate()) {
    output_frame_rate = get_output_frame_rate();
  } else {
    output_frame_rate = input_frame_rate;
  }

  bool all_ok = true;

  if (_from_selection) {
    all_ok = _tree.build_selected_hierarchy();
  } else {
    all_ok = _tree.build_complete_hierarchy();
  }

  if (all_ok) {
    switch (get_animation_convert()) {
    case AC_pose:
      // pose: set to a specific frame, then get out the static geometry.
      mayaegg_cat.info(false)
        << "frame " << start_frame << "\n";
      MGlobal::viewFrame(MTime(start_frame, MTime::uiUnit()));
      // fall through
      
    case AC_none:
      // none: just get out a static model, no animation.
      all_ok = convert_hierarchy(&get_egg_data());
      break;
      
    case AC_flip:
    case AC_strobe:
      // flip or strobe: get out a series of static models, one per
      // frame, under a sequence node for AC_flip.
      all_ok = convert_flip(start_frame, end_frame, frame_inc,
                            output_frame_rate);
      break;

    case AC_model:
      // model: get out an animatable model with joints and vertex
      // membership.
      all_ok = convert_char_model();
      break;

    case AC_chan:
      // chan: get out a series of animation tables.
      all_ok = convert_char_chan(start_frame, end_frame, frame_inc,
                                 output_frame_rate);
      break;
      
    case AC_both:
      // both: Put a model and its animation into the same egg file.
      _animation_convert = AC_model;
      if (!convert_char_model()) {
        all_ok = false;
      }
      _animation_convert = AC_chan;
      if (!convert_char_chan(start_frame, end_frame, frame_inc,
                             output_frame_rate)) {
        all_ok = false;
      }
      break;
    };

    reparent_decals(&get_egg_data());
  }

  if (all_ok) {
    mayaegg_cat.info()
      << "Converted, no errors.\n";
  } else {
    mayaegg_cat.info()
      << "Errors encountered in conversion.\n";
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::open_api
//       Access: Public
//  Description: Attempts to open the Maya API if it was not already
//               open, and returns true if successful, or false if
//               there is an error.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
open_api() {
  if (_maya == (MayaApi *)NULL || !_maya->is_valid()) {
    _maya = MayaApi::open_api(_program_name);
  }
  return _maya->is_valid();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::close_api
//       Access: Public
//  Description: Closes the Maya API, if it was previously opened.
//               Caution!  Maya appears to call exit() when its API is
//               closed.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
close_api() {
  // We have to clear the shaders before we release the Maya API.
  _shaders.clear();
  _maya.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_flip
//       Access: Private
//  Description: Converts the animation as a series of models that
//               cycle (flip) from one to the next at the appropriate
//               frame rate.  This is the most likely to convert
//               precisely (since we ask Maya to tell us the vertex
//               position each time) but it is the most wasteful in
//               terms of memory utilization (since a complete of the
//               model is stored for each frame).
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_flip(double start_frame, double end_frame, double frame_inc,
             double output_frame_rate) {
  bool all_ok = true;

  EggGroup *sequence_node = new EggGroup(_character_name);
  get_egg_data().add_child(sequence_node);
  if (_animation_convert == AC_flip) { 
    sequence_node->set_switch_flag(true);
    sequence_node->set_switch_fps(output_frame_rate / frame_inc);
  }

  MTime frame(start_frame, MTime::uiUnit());
  MTime frame_stop(end_frame, MTime::uiUnit());
  while (frame <= frame_stop) {
    mayaegg_cat.info(false)
      << "frame " << frame.value() << "\n";
    ostringstream name_strm;
    name_strm << "frame" << frame.value();
    EggGroup *frame_root = new EggGroup(name_strm.str());
    sequence_node->add_child(frame_root);

    MGlobal::viewFrame(frame);
    if (!convert_hierarchy(frame_root)) {
      all_ok = false;
    }

    frame += frame_inc;
  }

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_char_model
//       Access: Private
//  Description: Converts the file as an animatable character
//               model, with joints and vertex membership.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_char_model() {
  if (has_neutral_frame()) {
    MTime frame(get_neutral_frame(), MTime::uiUnit());
    mayaegg_cat.info(false)
      << "neutral frame " << frame.value() << "\n";
    MGlobal::viewFrame(frame);
  }

  EggGroup *char_node = new EggGroup(_character_name);
  get_egg_data().add_child(char_node);
  char_node->set_dart_type(EggGroup::DT_default);

  return convert_hierarchy(char_node);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_char_chan
//       Access: Private
//  Description: Converts the animation as a series of tables to apply
//               to the character model, as retrieved earlier via
//               AC_model.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_char_chan(double start_frame, double end_frame, double frame_inc,
                  double output_frame_rate) {
  MStatus status;

  EggTable *root_table_node = new EggTable();
  get_egg_data().add_child(root_table_node);
  EggTable *bundle_node = new EggTable(_character_name);
  bundle_node->set_table_type(EggTable::TT_bundle);
  root_table_node->add_child(bundle_node);
  EggTable *skeleton_node = new EggTable("<skeleton>");
  bundle_node->add_child(skeleton_node);

  // Set the frame rate before we start asking for anim tables to be
  // created.
  _tree._fps = output_frame_rate / frame_inc;
  _tree.clear_egg(&get_egg_data(), NULL, skeleton_node);

  // Now we can get the animation data by walking through all of the
  // frames, one at a time, and getting the joint angles at each
  // frame.

  // This is just a temporary EggGroup to receive the transform for
  // each joint each frame.
  PT(EggGroup) tgroup = new EggGroup;

  int num_nodes = _tree.get_num_nodes();
  int i;

  MTime frame(start_frame, MTime::uiUnit());
  MTime frame_stop(end_frame, MTime::uiUnit());
  while (frame <= frame_stop) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug(false)
        << "frame " << frame.value() << "\n";
    } else {
      // We have to write to cerr instead of mayaegg_cat to allow
      // flushing without writing a newline.
      cerr << "." << flush;
    }
    MGlobal::viewFrame(frame);

    for (i = 0; i < num_nodes; i++) {
      MayaNodeDesc *node_desc = _tree.get_node(i);
      if (node_desc->is_joint()) {
        if (mayaegg_cat.is_spam()) {
          mayaegg_cat.spam()
            << "joint " << node_desc->get_name() << "\n";
        }
        get_joint_transform(node_desc->get_dag_path(), tgroup);
        EggXfmSAnim *anim = _tree.get_egg_anim(node_desc);
        if (!anim->add_data(tgroup->get_transform())) {
          mayaegg_cat.error()
            << "Invalid transform on " << node_desc->get_name()
            << " frame " << frame.value() << ".\n";
        }
      }
    }

    frame += frame_inc;
  }

  // Now optimize all of the tables we just filled up, for no real
  // good reason, except that it makes the resulting egg file a little
  // easier to read.
  for (i = 0; i < num_nodes; i++) {
    MayaNodeDesc *node_desc = _tree.get_node(i);
    if (node_desc->is_joint()) {
      _tree.get_egg_anim(node_desc)->optimize();
    }
  }

  mayaegg_cat.info(false)
    << "\n";

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_hierarchy
//       Access: Private
//  Description: Generates egg structures for each node in the Maya
//               hierarchy.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_hierarchy(EggGroupNode *egg_root) {
  int num_nodes = _tree.get_num_nodes();

  _tree.clear_egg(&get_egg_data(), egg_root, NULL);
  for (int i = 0; i < num_nodes; i++) {
    if (!process_model_node(_tree.get_node(i))) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::process_model_node
//       Access: Private
//  Description: Converts the indicated Maya node (given a MDagPath,
//               similar in concept to Panda's NodePath) to the
//               corresponding Egg structure.  Returns true if
//               successful, false if an error was encountered.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
process_model_node(MayaNodeDesc *node_desc) {
  if (!node_desc->has_dag_path()) {
    // If the node has no Maya equivalent, never mind.
    return true;
  }

  MDagPath dag_path = node_desc->get_dag_path();

  MStatus status;
  MFnDagNode dag_node(dag_path, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return false;
  }

  MObject node = dag_path.transform(&status);

  bool visible;
  if (!get_bool_attribute(node, "visibility", visible)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Couldn't get visibility attribute for " << dag_node.name()
        << "\n";
    }
    visible = true;
  }

  string path = dag_path.fullPathName().asChar();

  if (mayaegg_cat.is_debug()) {
    mayaegg_cat.debug()
      << path << ": " << dag_node.typeName();

    if (MAnimUtil::isAnimated(dag_path)) {
      mayaegg_cat.debug(false)
        << " (animated)";
    }

    if (!visible) {
      mayaegg_cat.debug(false)
        << " (invisible)";
    }

    mayaegg_cat.debug(false) << "\n";
  }

  if (node_desc->is_joint()) {
    // Don't bother with joints unless we're getting an animatable
    // model.
    if (_animation_convert == AC_model) { 
      EggGroup *egg_group = _tree.get_egg_group(node_desc);
      get_joint_transform(dag_path, egg_group);
    }

  } else if (!visible) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring invisible node " << path
        << "\n";
    }

  } else if (dag_node.inUnderWorld()) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring underworld node " << path
        << "\n";
    }

  } else if (dag_node.isIntermediateObject()) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring intermediate object " << path
        << "\n";
    }

  } else if (dag_path.hasFn(MFn::kCamera)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring camera node " << path
        << "\n";
    }

  } else if (dag_path.hasFn(MFn::kLight)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring light node " << path
        << "\n";
    }

  } else if (dag_path.hasFn(MFn::kNurbsSurface)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(dag_path, egg_group);
    
    MFnNurbsSurface surface(dag_path, &status);
    if (!status) {
      mayaegg_cat.info()
        << "Error in node " << path
        << ":\n"
        << "  it appears to have a NURBS surface, but does not.\n";
    } else {
      make_nurbs_surface(dag_path, surface, egg_group);
    }

  } else if (dag_path.hasFn(MFn::kNurbsCurve)) {
    // Only convert NurbsCurves if we aren't making an animated model.
    // Animated models, as a general rule, don't want these sorts of
    // things in them.
    if (_animation_convert != AC_model) {
      EggGroup *egg_group = _tree.get_egg_group(node_desc);
      get_transform(dag_path, egg_group);
      
      MFnNurbsCurve curve(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << path << ":\n"
          << "  it appears to have a NURBS curve, but does not.\n";
      } else {
        make_nurbs_curve(dag_path, curve, egg_group);
      }
    }
      
  } else if (dag_path.hasFn(MFn::kMesh)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(dag_path, egg_group);

    MFnMesh mesh(dag_path, &status);
    if (!status) {
      mayaegg_cat.info()
        << "Error in node " << path << ":\n"
        << "  it appears to have a polygon mesh, but does not.\n";
    } else {
      make_polyset(dag_path, mesh, egg_group);
    }

  } else if (dag_path.hasFn(MFn::kLocator)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);

    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Locator at " << path << "\n";
    }
    
    // Presumably, the locator's position has some meaning to the
    // end-user, so we will implicitly tag it with the DCS flag so it
    // won't get flattened out.
    if (_animation_convert != AC_model) {
      // For now, don't set the DCS flag on locators within
      // character models, since egg-optchar doesn't understand
      // this.  Perhaps there's no reason to ever change this, since
      // locators within character models may not be meaningful.
      egg_group->set_dcs_type(EggGroup::DC_net);
    }
    get_transform(dag_path, egg_group);
    make_locator(dag_path, dag_node, egg_group);

  } else {
    // Just a generic node.
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(dag_path, egg_group);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               and applies it to the corresponding Egg node.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
get_transform(const MDagPath &dag_path, EggGroup *egg_group) {
  if (_animation_convert == AC_model) {
    // When we're getting an animated model, we only get transforms
    // for joints.
    return;
  }

  MStatus status;
  MObject transformNode = dag_path.transform(&status);
  if (!status && status.statusCode() == MStatus::kInvalidParameter) {
    // This node has no transform - i.e., it's the world node
    return;
  }

  // Billboards always get the transform set.
  if (egg_group->get_billboard_type() == EggGroup::BT_none) {
    switch (_transform_type) {
    case TT_all:
      break;
      
    case TT_model:
      if (!egg_group->get_model_flag() &&
          egg_group->get_dcs_type() == EggGroup::DC_none) {
        return;
      }
      break;
      
    case TT_dcs: 
      if (egg_group->get_dcs_type() == EggGroup::DC_none) {
        return;
      }
      break;
      
    case TT_none:
    case TT_invalid:
      return;
    }
  }

  // Extract the matrix from the dag path.
  MMatrix mat = dag_path.inclusiveMatrix(&status);
  if (!status) {
    status.perror("Can't get transform matrix");
    return;
  }
  LMatrix4d m4d(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);

  // Maya has a rotate pivot, separate from its transform.  Usually we
  // care more about the rotate pivot than we do about the transform,
  // so get the rotate pivot too.
  MFnTransform transform(transformNode, &status);
  if (!status) {
    status.perror("MFnTransform constructor");
    return;
  }
  MPoint pivot = transform.rotatePivot(MSpace::kObject, &status);
  if (!status) {
    status.perror("Can't get rotate pivot");
    return;
  }
  
  // We need to convert the pivot to world coordinates.  (Maya can
  // only tell it to us in local coordinates.)
  LPoint3d p3d(pivot[0], pivot[1], pivot[2]);
  p3d = p3d * m4d;

  // Now recenter the matrix about the pivot point.
  m4d.set_row(3, p3d);

  // Convert the recentered matrix into the group's space and store
  // it.
  m4d = m4d * egg_group->get_node_frame_inv();
  if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
    egg_group->add_matrix(m4d);
  }
  return;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_joint_transform
//       Access: Private
//  Description: Extracts the transform on the indicated Maya node,
//               as appropriate for a joint in an animated character,
//               and applies it to the indicated node.  This is
//               different from get_transform() in that it does not
//               respect the _transform_type flag, and it does not
//               consider the relative transforms within the egg file.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
get_joint_transform(const MDagPath &dag_path, EggGroup *egg_group) {
  MStatus status;
  MObject transformNode = dag_path.transform(&status);
  // This node has no transform - i.e., it's the world node
  if (!status && status.statusCode() == MStatus::kInvalidParameter) {
    return;
  }

  MFnDagNode transform(transformNode, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return;
  }

  MTransformationMatrix matrix(transform.transformationMatrix());

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  translation: " << matrix.translation(MSpace::kWorld)
      << "\n";
    double d[3];
    MTransformationMatrix::RotationOrder rOrder;

    matrix.getRotation(d, rOrder, MSpace::kWorld);
    mayaegg_cat.spam()
      << "  rotation: ["
      << d[0] << ", "
      << d[1] << ", "
      << d[2] << "]\n";
    matrix.getScale(d, MSpace::kWorld);
    mayaegg_cat.spam()
      << "  scale: ["
      << d[0] << ", "
      << d[1] << ", "
      << d[2] << "]\n";
    matrix.getShear(d, MSpace::kWorld);
    mayaegg_cat.spam()
      << "  shear: ["
      << d[0] << ", "
      << d[1] << ", "
      << d[2] << "]\n";
  }

  MMatrix mat = matrix.asMatrix();
  MMatrix ident_mat;
  ident_mat.setToIdentity();

  if (!mat.isEquivalent(ident_mat, 0.0001)) {
    egg_group->set_transform
      (LMatrix4d(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                 mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                 mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                 mat[3][0], mat[3][1], mat[3][2], mat[3][3]));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_nurbs_surface
//       Access: Private
//  Description: Converts the indicated Maya NURBS surface to a
//               corresponding egg structure, and attaches it to the
//               indicated egg group.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_nurbs_surface(const MDagPath &dag_path, MFnNurbsSurface &surface,
                   EggGroup *egg_group) {
  MStatus status;
  string name = surface.name().asChar();

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  numCVs: "
      << surface.numCVsInU()
      << " * "
      << surface.numCVsInV()
      << "\n";
    mayaegg_cat.spam()
      << "  numKnots: "
      << surface.numKnotsInU()
      << " * "
      << surface.numKnotsInV()
      << "\n";
    mayaegg_cat.spam()
      << "  numSpans: "
      << surface.numSpansInU()
      << " * "
      << surface.numSpansInV()
      << "\n";
  }

  MayaShader *shader = _shaders.find_shader_for_node(surface.object());

  if (_polygon_output) {
    // If we want polygon output only, tesselate the NURBS and output
    // that.
    MTesselationParams params;
    params.setFormatType(MTesselationParams::kStandardFitFormat);
    params.setOutputType(MTesselationParams::kQuads);
    params.setStdFractionalTolerance(_polygon_tolerance);

    // We'll create the tesselation as a sibling of the NURBS surface.
    // That way we inherit all of the transformations.
    MDagPath polyset_path = dag_path;
    MObject polyset_parent = polyset_path.node();
    MObject polyset =
      surface.tesselate(params, polyset_parent, &status);
    if (!status) {
      status.perror("MFnNurbsSurface::tesselate");
      return;
    }

    status = polyset_path.push(polyset);
    if (!status) {
      status.perror("MDagPath::push");
    }

    MFnMesh polyset_fn(polyset, &status);
    if (!status) {
      status.perror("MFnMesh constructor");
      return;
    }
    make_polyset(polyset_path, polyset_fn, egg_group, shader);

    // Now remove the polyset we created.
    MFnDagNode parent_node(polyset_parent, &status);
    if (!status) {
      status.perror("MFnDagNode constructor");
      return;
    }
    status = parent_node.removeChild(polyset);
    if (!status) {
      status.perror("MFnDagNode::removeChild");
    }

    return;
  }

  MPointArray cv_array;
  status = surface.getCVs(cv_array, MSpace::kWorld);
  if (!status) {
    status.perror("MFnNurbsSurface::getCVs");
    return;
  }
  MDoubleArray u_knot_array, v_knot_array;
  status = surface.getKnotsInU(u_knot_array);
  if (!status) {
    status.perror("MFnNurbsSurface::getKnotsInU");
    return;
  }
  status = surface.getKnotsInV(v_knot_array);
  if (!status) {
    status.perror("MFnNurbsSurface::getKnotsInV");
    return;
  }

  /*
    We don't use these variables currently.
  MFnNurbsSurface::Form u_form = surface.formInU();
  MFnNurbsSurface::Form v_form = surface.formInV();
  */

  int u_degree = surface.degreeU();
  int v_degree = surface.degreeV();

  int u_cvs = surface.numCVsInU();
  int v_cvs = surface.numCVsInV();

  int u_knots = surface.numKnotsInU();
  int v_knots = surface.numKnotsInV();

  assert(u_knots == u_cvs + u_degree - 1);
  assert(v_knots == v_cvs + v_degree - 1);

  string vpool_name = name + ".cvs";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsSurface *egg_nurbs = new EggNurbsSurface(name);
  egg_nurbs->setup(u_degree + 1, v_degree + 1,
                   u_knots + 2, v_knots + 2);

  int i;

  egg_nurbs->set_u_knot(0, u_knot_array[0]);
  for (i = 0; i < u_knots; i++) {
    egg_nurbs->set_u_knot(i + 1, u_knot_array[i]);
  }
  egg_nurbs->set_u_knot(u_knots + 1, u_knot_array[u_knots - 1]);

  egg_nurbs->set_v_knot(0, v_knot_array[0]);
  for (i = 0; i < v_knots; i++) {
    egg_nurbs->set_v_knot(i + 1, v_knot_array[i]);
  }
  egg_nurbs->set_v_knot(v_knots + 1, v_knot_array[v_knots - 1]);

  LMatrix4d vertex_frame_inv = egg_group->get_vertex_frame_inv();

  for (i = 0; i < egg_nurbs->get_num_cvs(); i++) {
    int ui = egg_nurbs->get_u_index(i);
    int vi = egg_nurbs->get_v_index(i);

    double v[4];
    MStatus status = cv_array[v_cvs * ui + vi].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      LPoint4d p4d(v[0], v[1], v[2], v[3]);
      p4d = p4d * vertex_frame_inv;
      vert.set_pos(p4d);
      egg_nurbs->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  // Now consider the trim curves, if any.
  unsigned num_trims = surface.numRegions();
  int trim_curve_index = 0;
  for (unsigned ti = 0; ti < num_trims; ti++) {
    unsigned num_loops = surface.numBoundaries(ti);

    if (num_loops > 0) {
      egg_nurbs->_trims.push_back(EggNurbsSurface::Trim());
      EggNurbsSurface::Trim &egg_trim = egg_nurbs->_trims.back();

      for (unsigned li = 0; li < num_loops; li++) {
        egg_trim.push_back(EggNurbsSurface::Loop());
        EggNurbsSurface::Loop &egg_loop = egg_trim.back();
        
        MFnNurbsSurface::BoundaryType type =
          surface.boundaryType(ti, li, &status);
        bool keep_loop = false;
        
        if (!status) {
          status.perror("MFnNurbsSurface::BoundaryType");
        } else {
          keep_loop = (type == MFnNurbsSurface::kInner ||
                       type == MFnNurbsSurface::kOuter);
        }
        
        if (keep_loop) {
          unsigned num_edges = surface.numEdges(ti, li);
          for (unsigned ei = 0; ei < num_edges; ei++) {
            MObjectArray edge = surface.edge(ti, li, ei, true, &status);
            if (!status) {
              status.perror("MFnNurbsSurface::edge");
            } else {
              unsigned num_segs = edge.length();
              for (unsigned si = 0; si < num_segs; si++) {
                MObject segment = edge[si];
                if (segment.hasFn(MFn::kNurbsCurve)) {
                  MFnNurbsCurve curve(segment, &status);
                  if (!status) {
                    mayaegg_cat.error()
                      << "Trim curve appears to be a nurbs curve, but isn't.\n";
                  } else {
                    // Finally, we have a valid curve!
                    EggNurbsCurve *egg_curve =
                      make_trim_curve(curve, name, egg_group, trim_curve_index);
                    trim_curve_index++;
                    if (egg_curve != (EggNurbsCurve *)NULL) {
                      egg_loop.push_back(egg_curve);
                    }
                  }
                } else {
                  mayaegg_cat.error()
                    << "Trim curve segment is not a nurbs curve.\n";
                }
              }
            }
          }
        }
      }
    }
  }

  // We add the NURBS to the group down here, after all of the vpools
  // for the trim curves have been added.
  egg_group->add_child(egg_nurbs);

  if (shader != (MayaShader *)NULL) {
    set_shader_attributes(*egg_nurbs, *shader);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_trim_curve
//       Access: Private
//  Description: Converts the indicated Maya NURBS trim curve to a
//               corresponding egg structure, and returns it, or NULL
//               if there is a problem.
////////////////////////////////////////////////////////////////////
EggNurbsCurve *MayaToEggConverter::
make_trim_curve(const MFnNurbsCurve &curve, const string &nurbs_name,
                EggGroupNode *egg_group, int trim_curve_index) {
  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "Trim curve:\n";
    mayaegg_cat.spam()
      << "  numCVs: "
      << curve.numCVs()
      << "\n";
    mayaegg_cat.spam()
      << "  numKnots: "
      << curve.numKnots()
      << "\n";
    mayaegg_cat.spam()
      << "  numSpans: "
      << curve.numSpans()
      << "\n";
  }

  MStatus status;

  MPointArray cv_array;
  status = curve.getCVs(cv_array, MSpace::kWorld);
  if (!status) {
    status.perror("MFnNurbsCurve::getCVs");
    return (EggNurbsCurve *)NULL;
  }
  MDoubleArray knot_array;
  status = curve.getKnots(knot_array);
  if (!status) {
    status.perror("MFnNurbsCurve::getKnots");
    return (EggNurbsCurve *)NULL;
  }

  /*
  MFnNurbsCurve::Form form = curve.form();
  */

  int degree = curve.degree();
  int cvs = curve.numCVs();
  int knots = curve.numKnots();

  assert(knots == cvs + degree - 1);

  string trim_name = "trim" + format_string(trim_curve_index);

  string vpool_name = nurbs_name + "." + trim_name;
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsCurve *egg_curve = new EggNurbsCurve(trim_name);
  egg_curve->setup(degree + 1, knots + 2);

  int i;

  egg_curve->set_knot(0, knot_array[0]);
  for (i = 0; i < knots; i++) {
    egg_curve->set_knot(i + 1, knot_array[i]);
  }
  egg_curve->set_knot(knots + 1, knot_array[knots - 1]);

  for (i = 0; i < egg_curve->get_num_cvs(); i++) {
    double v[4];
    MStatus status = cv_array[i].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      vert.set_pos(LPoint3d(v[0], v[1], v[3]));
      egg_curve->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  return egg_curve;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_nurbs_curve
//       Access: Private
//  Description: Converts the indicated Maya NURBS curve (a standalone
//               curve, not a trim curve) to a corresponding egg
//               structure and attaches it to the indicated egg group.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_nurbs_curve(const MDagPath &, const MFnNurbsCurve &curve,
                 EggGroup *egg_group) {
  MStatus status;
  string name = curve.name().asChar();

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  numCVs: "
      << curve.numCVs()
      << "\n";
    mayaegg_cat.spam()
      << "  numKnots: "
      << curve.numKnots()
      << "\n";
    mayaegg_cat.spam()
      << "  numSpans: "
      << curve.numSpans()
      << "\n";
  }

  MPointArray cv_array;
  status = curve.getCVs(cv_array, MSpace::kWorld);
  if (!status) {
    status.perror("MFnNurbsCurve::getCVs");
    return;
  }
  MDoubleArray knot_array;
  status = curve.getKnots(knot_array);
  if (!status) {
    status.perror("MFnNurbsCurve::getKnots");
    return;
  }

  /*
  MFnNurbsCurve::Form form = curve.form();
  */

  int degree = curve.degree();
  int cvs = curve.numCVs();
  int knots = curve.numKnots();

  assert(knots == cvs + degree - 1);

  string vpool_name = name + ".cvs";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  EggNurbsCurve *egg_curve = new EggNurbsCurve(name);
  egg_group->add_child(egg_curve);
  egg_curve->setup(degree + 1, knots + 2);

  int i;

  egg_curve->set_knot(0, knot_array[0]);
  for (i = 0; i < knots; i++) {
    egg_curve->set_knot(i + 1, knot_array[i]);
  }
  egg_curve->set_knot(knots + 1, knot_array[knots - 1]);

  LMatrix4d vertex_frame_inv = egg_group->get_vertex_frame_inv();

  for (i = 0; i < egg_curve->get_num_cvs(); i++) {
    double v[4];
    MStatus status = cv_array[i].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      LPoint4d p4d(v[0], v[1], v[2], v[3]);
      p4d = p4d * vertex_frame_inv;
      vert.set_pos(p4d);
      egg_curve->add_vertex(vpool->create_unique_vertex(vert));
    }
  }

  MayaShader *shader = _shaders.find_shader_for_node(curve.object());
  if (shader != (MayaShader *)NULL) {
    set_shader_attributes(*egg_curve, *shader);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_polyset
//       Access: Private
//  Description: Converts the indicated Maya polyset to a bunch of
//               EggPolygons and parents them to the indicated egg
//               group.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_polyset(const MDagPath &dag_path, const MFnMesh &mesh,
             EggGroup *egg_group, MayaShader *default_shader) {
  MStatus status;
  string name = mesh.name().asChar();

  MObject mesh_object = mesh.object();
  bool maya_double_sided = false;
  get_bool_attribute(mesh_object, "doubleSided", maya_double_sided);

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam()
      << "  numPolygons: "
      << mesh.numPolygons()
      << "\n";
    mayaegg_cat.spam()
      << "  numVertices: "
      << mesh.numVertices()
      << "\n";
  }

  if (mesh.numPolygons() == 0) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring empty mesh " << name << "\n";
    }
    return;
  }

  string vpool_name = name + ".verts";
  EggVertexPool *vpool = new EggVertexPool(vpool_name);
  egg_group->add_child(vpool);

  // One way to convert the mesh would be to first get out all the
  // vertices in the mesh and add them into the vpool, then when we
  // traverse the polygons we would only have to index them into the
  // vpool according to their Maya vertex index.

  // Unfortunately, since Maya may store multiple normals and/or
  // colors for each vertex according to which polygon it is in, that
  // approach won't necessarily work.  In egg, those split-property
  // vertices have to become separate vertices.  So instead of adding
  // all the vertices up front, we'll start with an empty vpool, and
  // add vertices to it on the fly.

  MObject component_obj;
  MItMeshPolygon pi(dag_path, component_obj, &status);
  if (!status) {
    status.perror("MItMeshPolygon constructor");
    return;
  }

  MObjectArray shaders;
  MIntArray poly_shader_indices;

  status = mesh.getConnectedShaders(dag_path.instanceNumber(),
                                    shaders, poly_shader_indices);
  if (!status) {
    status.perror("MFnMesh::getConnectedShaders");
  }

  // We will need to transform all vertices from world coordinate
  // space into the vertex space appropriate to this node.  Usually,
  // this is the same thing as world coordinate space, and this matrix
  // will be identity; but if the node is under an instance
  // (particularly, for instance, a billboard) then the vertex space
  // will be different from world space.
  LMatrix4d vertex_frame_inv = egg_group->get_vertex_frame_inv();

  // Save these modeling flags for the check below.
  bool egg_vertex_color = false;
  bool egg_double_sided = false;
  if (egg_group->has_user_data(MayaEggGroupUserData::get_class_type())) {
    MayaEggGroupUserData *user_data = 
      DCAST(MayaEggGroupUserData, egg_group->get_user_data());
    egg_vertex_color = user_data->_vertex_color;
    egg_double_sided = user_data->_double_sided;
  }

  bool double_sided = _respect_maya_double_sided ? maya_double_sided : egg_double_sided;

  while (!pi.isDone()) {
    EggPolygon *egg_poly = new EggPolygon;
    egg_group->add_child(egg_poly);

    egg_poly->set_bface_flag(double_sided);

    // Determine the shader for this particular polygon.
    MayaShader *shader = NULL;
    int index = pi.index();
    nassertv(index >= 0 && index < (int)poly_shader_indices.length());
    int shader_index = poly_shader_indices[index];
    if (shader_index != -1) {
      nassertv(shader_index >= 0 && shader_index < (int)shaders.length());
      MObject engine = shaders[shader_index];
      shader =
        _shaders.find_shader_for_shading_engine(engine);

    } else if (default_shader != (MayaShader *)NULL) {
      shader = default_shader;
    }

    const MayaShaderColorDef &color_def = shader->_color;

    // Should we extract the color from the vertices?  Normally, in
    // Maya a texture completely replaces the vertex color, so we
    // should ignore the vertex color if we have a texture.

    // However, this is an inconvenient property of Maya; sometimes we
    // really do want both vertex color and texture applied to the
    // same object.  To allow this, we define the special egg flag
    // "vertex-color", which when set indicates that we should
    // respect the vertex color anyway.

    // Furthermore, if _always_show_vertex_color is true, we pretend
    // that the "vertex-color" flag is always set.
    bool ignore_vertex_color = false;
    if (shader != (MayaShader *)NULL) {
      ignore_vertex_color = color_def._has_texture && !(egg_vertex_color || _always_show_vertex_color);
    }

    // Get the vertices for the polygon.
    long num_verts = pi.polygonVertexCount();
    long i;
    LPoint3d centroid(0.0, 0.0, 0.0);

    if (shader != (MayaShader *)NULL && color_def.has_projection()) {
      // If the shader has a projection, we may need to compute the
      // polygon's centroid to avoid seams at the edges.
      for (i = 0; i < num_verts; i++) {
        MPoint p = pi.point(i, MSpace::kWorld);
        LPoint3d p3d(p[0], p[1], p[2]);
        p3d = p3d * vertex_frame_inv;
        centroid += p3d;
      }
      centroid /= (double)num_verts;
    }

    for (i = 0; i < num_verts; i++) {
      EggVertex vert;

      MPoint p = pi.point(i, MSpace::kWorld);
      LPoint3d p3d(p[0], p[1], p[2]);
      p3d = p3d * vertex_frame_inv;
      vert.set_pos(p3d);

      MVector n;
      status = pi.getNormal(i, n, MSpace::kWorld);
      if (!status) {
        status.perror("MItMeshPolygon::getNormal");
      } else {
        LVector3d n3d(n[0], n[1], n[2]);
        n3d = n3d * vertex_frame_inv;
        vert.set_normal(n3d);
      }

      if (shader != (MayaShader *)NULL && color_def.has_projection()) {
        // If the shader has a projection, use it instead of the
        // polygon's built-in UV's.
        vert.set_uv(color_def.project_uv(p3d, centroid));

      } else if (pi.hasUVs()) {
        // Get the UV's from the polygon.
        float2 uvs;
        status = pi.getUV(i, uvs);
        if (!status) {
          status.perror("MItMeshPolygon::getUV");
        } else {
          vert.set_uv(TexCoordd(uvs[0], uvs[1]));
        }
      }

      if (pi.hasColor() && !ignore_vertex_color) {
        MColor c;
        status = pi.getColor(c, i);
        if (!status) {
          status.perror("MItMeshPolygon::getColor");
        } else {
          vert.set_color(Colorf(c.r, c.g, c.b, 1.0));
        }
      }

      vert.set_external_index(pi.vertexIndex(i, &status));

      egg_poly->add_vertex(vpool->create_unique_vertex(vert));
    }

    // Now apply the shader.
    if (shader != (MayaShader *)NULL) {
      set_shader_attributes(*egg_poly, *shader);
    }

    pi.next();
  }

  // Now that we've added all the polygons (and created all the
  // vertices), go back through the vertex pool and set up the
  // appropriate joint membership for each of the vertices.
  bool got_weights = false;

  pvector<EggGroup *> joints;
  MFloatArray weights;
  if (_animation_convert == AC_model) {
    got_weights = 
      get_vertex_weights(dag_path, mesh, joints, weights);
  }

  if (got_weights && !joints.empty()) {
    int num_joints = joints.size();
    int num_weights = (int)weights.length();
    int num_verts = num_weights / num_joints;
    // The number of weights should be an even multiple of verts *
    // joints.
    nassertv(num_weights == num_verts * num_joints);

    EggVertexPool::iterator vi;
    for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
      EggVertex *vert = (*vi);
      int maya_vi = vert->get_external_index();
      nassertv(maya_vi >= 0 && maya_vi < num_verts);

      for (int ji = 0; ji < num_joints; ++ji) {
        float weight = weights[maya_vi * num_joints + ji];
        if (weight != 0.0f) {
          EggGroup *joint = joints[ji];
          if (joint != (EggGroup *)NULL) {
            joint->ref_vertex(vert, weight);
          }
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::make_locator
//       Access: Private
//  Description: Locators are used in Maya to indicate a particular
//               position in space to the user or the modeler.  We
//               represent that in egg with an ordinary Group node,
//               which we transform by the locator's position, so that
//               the indicated point becomes the origin at this node
//               and below.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
make_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
             EggGroup *egg_group) {
  MStatus status;

  unsigned int num_children = dag_node.childCount();
  MObject locator;
  bool found_locator = false;
  for (unsigned int ci = 0; ci < num_children && !found_locator; ci++) {
    locator = dag_node.child(ci);
    found_locator = (locator.apiType() == MFn::kLocator);
  }

  if (!found_locator) {
    mayaegg_cat.error()
      << "Couldn't find locator within locator node " 
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  LPoint3d p3d;
  if (!get_vec3d_attribute(locator, "localPosition", p3d)) {
    mayaegg_cat.error()
      << "Couldn't get position of locator " 
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  // We need to convert the position to world coordinates.  For some
  // reason, Maya can only tell it to us in local coordinates.
  MMatrix mat = dag_path.inclusiveMatrix(&status);
  if (!status) {
    status.perror("Can't get coordinate space for locator");
    return;
  }
  LMatrix4d n2w(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
  p3d = p3d * n2w;

  // Now convert the locator point into the group's space.
  p3d = p3d * egg_group->get_node_frame_inv();

  egg_group->add_translate(p3d);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_vertex_weights
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                   pvector<EggGroup *> &joints, MFloatArray &weights) {
  MStatus status;
  
  // Since we are working with a mesh the input attribute that 
  // creates the mesh is named "inMesh" 
  // 
  MObject attr = mesh.attribute("inMesh"); 
  
  // Create the plug to the "inMesh" attribute then use the 
  // DG iterator to walk through the DG, at the node level.
  // 
  MPlug history(mesh.object(), attr); 
  MItDependencyGraph it(history, MFn::kDependencyNode, 
                        MItDependencyGraph::kUpstream, 
                        MItDependencyGraph::kDepthFirst, 
                        MItDependencyGraph::kNodeLevel);

  while (!it.isDone()) {
    // We will walk along the node level of the DG until we 
    // spot a skinCluster node.
    // 
    MObject c_node = it.thisNode(); 
    if (c_node.hasFn(MFn::kSkinClusterFilter)) { 
      // We've found the cluster handle. Try to get the weight
      // data.
      // 
      MFnSkinCluster cluster(c_node, &status); 
      if (!status) {
        status.perror("MFnSkinCluster constructor");
        return false;
      }

      // Get the set of objects that influence the vertices of this
      // mesh.  Hopefully these will all be joints.
      MDagPathArray influence_objects;
      cluster.influenceObjects(influence_objects, &status); 
      if (!status) {
        status.perror("MFnSkinCluster::influenceObjects");

      } else {
        // Fill up the vector with the corresponding table of egg
        // groups for each joint.
        joints.clear();
        for (unsigned oi = 0; oi < influence_objects.length(); oi++) {
          MDagPath joint_dag_path = influence_objects[oi];
          MayaNodeDesc *joint_node_desc = _tree.build_node(joint_dag_path);
          EggGroup *joint = _tree.get_egg_group(joint_node_desc);
          joints.push_back(joint);
        }

        // Now use a component object to retrieve all of the weight
        // data in one API call.
        MFnSingleIndexedComponent sic; 
        MObject sic_object = sic.create(MFn::kMeshVertComponent); 
        sic.setCompleteData(mesh.numVertices()); 
        unsigned influence_count; 

        status = cluster.getWeights(dag_path, sic_object, 
                                    weights, influence_count); 
        if (!status) {
          status.perror("MFnSkinCluster::getWeights");
        } else {
          if (influence_count != influence_objects.length()) {
            mayaegg_cat.error()
              << "MFnSkinCluster::influenceObjects() returns " 
              << influence_objects.length()
              << " objects, but MFnSkinCluster::getWeights() reports "
              << influence_count << " objects.\n";
            
          } else {
            // We've got the weights and the set of objects.  That's all
            // we need.
            return true;
          }
        }
      }
    }

    it.next();
  }
  
  mayaegg_cat.error()
    << "Unable to find a cluster handle for the DG node.\n"; 
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::set_shader_attributes
//       Access: Private
//  Description: Applies the known shader attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader) {
  // In Maya, a polygon is either textured or colored.  The texture,
  // if present, replaces the color.
  const MayaShaderColorDef &color_def = shader._color;
  const MayaShaderColorDef &trans_def = shader._transparency;
  if (color_def._has_texture || trans_def._has_texture) {
    EggTexture tex(shader.get_name(), "");

    if (color_def._has_texture) {
      // If we have a texture on color, apply it as the filename.
      Filename filename = Filename::from_os_specific(color_def._texture);
      Filename fullpath = 
        _path_replace->match_path(filename, get_texture_path());
      tex.set_filename(_path_replace->store_path(fullpath));
      tex.set_fullpath(fullpath);
      apply_texture_properties(tex, color_def);

      // If we also have a texture on transparency, apply it as the
      // alpha filename.
      if (trans_def._has_texture) {
        if (color_def._wrap_u != trans_def._wrap_u ||
            color_def._wrap_u != trans_def._wrap_u) {
          mayaegg_cat.warning()
            << "Shader " << shader.get_name()
            << " has contradictory wrap modes on color and texture.\n";
        }
          
        if (!compare_texture_properties(tex, trans_def)) {
          // Only report each broken shader once.
          static pset<string> bad_shaders;
          if (bad_shaders.insert(shader.get_name()).second) {
            mayaegg_cat.error()
              << "Color and transparency texture properties differ on shader "
              << shader.get_name() << "\n";
          }
        }
        tex.set_format(EggTexture::F_rgba);
          
        // We should try to be smarter about whether the transparency
        // value is connected to the texture's alpha channel or to its
        // grayscale channel.  However, I'm not sure how to detect
        // this at the moment; rather than spending days trying to
        // figure out, for now I'll just assume that if the same
        // texture image is used for both color and transparency, then
        // the artist meant to use the alpha channel for transparency.
        if (trans_def._texture == color_def._texture) {
          // That means that we don't need to do anything special: use
          // all the channels of the texture.

        } else {
          // Otherwise, pull the alpha channel from the other image
          // file.  Ideally, we should figure out which channel from
          // the other image supplies alpha (and specify this via
          // set_alpha_file_channel()), but for now we assume it comes
          // from the grayscale data.
          filename = Filename::from_os_specific(trans_def._texture);
          fullpath = _path_replace->match_path(filename, get_texture_path());
          tex.set_alpha_filename(_path_replace->store_path(fullpath));
          tex.set_alpha_fullpath(fullpath);
        }

      } else {
        // If there is no transparency texture specified, we don't
        // have any transparency, so tell the egg format to ignore any
        // alpha channel that might be on the color texture.
        tex.set_format(EggTexture::F_rgb);
      }

    } else {  // trans_def._has_texture
      // We have a texture on transparency only.  Apply it as the
      // primary filename, and set the format accordingly.
      Filename filename = Filename::from_os_specific(trans_def._texture);
      Filename fullpath = 
        _path_replace->match_path(filename, get_texture_path());
      tex.set_filename(_path_replace->store_path(fullpath));
      tex.set_fullpath(fullpath);
      tex.set_format(EggTexture::F_alpha);
      apply_texture_properties(tex, trans_def);
    }
  
    EggTexture *new_tex =
      _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);
    
    primitive.set_texture(new_tex);

  }

  // Also apply an overall color to the primitive.
  Colorf rgba = shader.get_rgba();

  // The existence of a texture on either color channel completely
  // replaces the corresponding flat color.
  if (color_def._has_texture) {
    rgba[0] = 1.0f;
    rgba[1] = 1.0f;
    rgba[2] = 1.0f;
  }
  if (trans_def._has_texture) {
    rgba[3] = 1.0f;
  }

  // But the color gain always gets applied.
  rgba[0] *= color_def._color_gain[0];
  rgba[1] *= color_def._color_gain[1];
  rgba[2] *= color_def._color_gain[2];
  rgba[3] *= color_def._color_gain[3];

  primitive.set_color(rgba);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::apply_texture_properties
//       Access: Private
//  Description: Applies all the appropriate texture properties to the
//               EggTexture object, including wrap modes and texture
//               matrix.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
apply_texture_properties(EggTexture &tex, const MayaShaderColorDef &color_def) {
  // Let's mipmap all textures by default.
  tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
  tex.set_magfilter(EggTexture::FT_linear);

  EggTexture::WrapMode wrap_u = color_def._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = color_def._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp;

  tex.set_wrap_u(wrap_u);
  tex.set_wrap_v(wrap_v);
  
  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(LMatrix3d::ident_mat())) {
    tex.set_transform(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::compare_texture_properties
//       Access: Private
//  Description: Compares the texture properties already on the
//               texture (presumably set by a previous call to
//               apply_texture_properties()) and returns false if they
//               differ from that specified by the indicated color_def
//               object, or true if they match.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
compare_texture_properties(EggTexture &tex, 
                           const MayaShaderColorDef &color_def) {
  bool okflag = true;

  EggTexture::WrapMode wrap_u = color_def._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = color_def._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  
  if (wrap_u != tex.determine_wrap_u()) {
    // Choose the more general of the two.
    if (wrap_u == EggTexture::WM_repeat) {
      tex.set_wrap_u(wrap_u);
    }
    okflag = false;
  }
  if (wrap_v != tex.determine_wrap_v()) {
    if (wrap_v == EggTexture::WM_repeat) {
      tex.set_wrap_v(wrap_v);
    }
    okflag = false;
  }
  
  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(tex.get_transform())) {
    okflag = false;
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::reparent_decals
//       Access: Private
//  Description: Recursively walks the egg hierarchy, reparenting
//               "decal" type nodes below their corresponding
//               "decalbase" type nodes, and setting the flags.
//
//               Returns true on success, false if some nodes were
//               incorrect.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
reparent_decals(EggGroupNode *egg_parent) {
  bool okflag = true;

  // First, walk through all children of this node, looking for the
  // one decal base, if any.
  EggGroup *decal_base = (EggGroup *)NULL;
  pvector<EggGroup *> decal_children;

  EggGroupNode::iterator ci;
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, child);
      if (child_group->has_object_type("decalbase")) {
        if (decal_base != (EggNode *)NULL) {
          mayaegg_cat.error()
            << "Two children of " << egg_parent->get_name()
            << " both have decalbase set: " << decal_base->get_name()
            << " and " << child_group->get_name() << "\n";
          okflag = false;
        }
        child_group->remove_object_type("decalbase");
        decal_base = child_group;

      } else if (child_group->has_object_type("decal")) {
        child_group->remove_object_type("decal");
        decal_children.push_back(child_group);
      }
    }
  }

  if (decal_base == (EggGroup *)NULL) {
    if (!decal_children.empty()) {
      mayaegg_cat.warning()
        << decal_children.front()->get_name()
        << " has decal, but no sibling node has decalbase.\n";
    }

  } else {
    if (decal_children.empty()) {
      mayaegg_cat.warning()
        << decal_base->get_name()
        << " has decalbase, but no sibling nodes have decal.\n";

    } else {
      // All the decal children get moved to be a child of decal base.
      // This usually will not affect the vertex positions, but it
      // could if the decal base has a transform and the decal child
      // is an instance node.  So don't do that.
      pvector<EggGroup *>::iterator di;
      for (di = decal_children.begin(); di != decal_children.end(); ++di) {
        EggGroup *child_group = (*di);
        decal_base->add_child(child_group);
      }

      // Also set the decal state on the base.
      decal_base->set_decal_flag(true);
    }
  }

  // Now recurse on each of the child nodes.
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *child_group = DCAST(EggGroupNode, child);
      if (!reparent_decals(child_group)) {
        okflag = false;
      }
    }
  }

  return okflag;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::string_transform_type
//       Access: Public, Static
//  Description: Returns the TransformType value corresponding to the
//               indicated string, or TT_invalid.
////////////////////////////////////////////////////////////////////
MayaToEggConverter::TransformType MayaToEggConverter::
string_transform_type(const string &arg) {
  if (cmp_nocase(arg, "all") == 0) {
    return TT_all;
  } else if (cmp_nocase(arg, "model") == 0) {
    return TT_model;
  } else if (cmp_nocase(arg, "dcs") == 0) {
    return TT_dcs;
  } else if (cmp_nocase(arg, "none") == 0) {
    return TT_none;
  } else {
    return TT_invalid;
  }
}
