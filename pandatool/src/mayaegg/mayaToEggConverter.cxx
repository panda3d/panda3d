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

#include "pre_maya_include.h"
#include <maya/MArgList.h>
#include <maya/MColor.h>
#include <maya/MDagPath.h>
#include <maya/MFnCamera.h>
#include <maya/MFnDagNode.h>
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
  _ignore_transforms = false;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
MayaToEggConverter::
MayaToEggConverter(const MayaToEggConverter &copy) :
  _maya(copy._maya)
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
  _groups.clear();
  _tables.clear();

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
    // flip: get out a series of static models, one per frame, under a
    // sequence node.
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
  sequence_node->set_switch_flag(true);
  sequence_node->set_switch_fps(output_frame_rate / frame_inc);

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
    _groups.clear();

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

  // First, walk through the scene graph and build up the table of
  // joints.
  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }
  bool all_ok = true;
  while (!dag_iterator.isDone()) {
    MDagPath dag_path;
    status = dag_iterator.getPath(dag_path);
    if (!status) {
      status.perror("MItDag::getPath");
    } else {
      if (!process_chan_node(dag_path, skeleton_node)) {
        all_ok = false;
      }
    }

    dag_iterator.next();
  }

  // Now walk through the joints in that table and make sure they're
  // all set to use the correct frame frame.
  double fps = output_frame_rate / frame_inc;
  Tables::iterator ti;
  for (ti = _tables.begin(); ti != _tables.end(); ++ti) {
    JointAnim *joint_anim = (*ti).second;
    nassertr(joint_anim != (JointAnim *)NULL && 
             joint_anim->_anim != (EggXfmSAnim *)NULL, false);
    joint_anim->_anim->set_fps(fps);
  }

  // Now we can get the animation data by walking through all of the
  // frames, one at a time, and getting the joint angles at each
  // frame.

  // This is just a temporary EggGroup to receive the transform for
  // each joint each frame.
  PT(EggGroup) tgroup = new EggGroup;

  MTime frame(start_frame, MTime::uiUnit());
  MTime frame_stop(end_frame, MTime::uiUnit());
  while (frame <= frame_stop) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug(false)
        << "frame " << frame.value() << "\n";
    } else {
      mayaegg_cat.info(false)
        << ".";
    }
    MGlobal::viewFrame(frame);

    for (ti = _tables.begin(); ti != _tables.end(); ++ti) {
      JointAnim *joint_anim = (*ti).second;
      get_transform(joint_anim->_dag_path, tgroup);
      joint_anim->_anim->add_data(tgroup->get_transform());
    }

    frame += frame_inc;
  }
  mayaegg_cat.info(false)
    << "\n";

  // Finally, clean up by deleting all of the JointAnim structures we
  // created.
  for (ti = _tables.begin(); ti != _tables.end(); ++ti) {
    JointAnim *joint_anim = (*ti).second;
    delete joint_anim;
  }
  _tables.clear();

  return all_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::convert_hierarchy
//       Access: Private
//  Description: Walks the entire Maya hierarchy, converting it to a
//               corresponding egg hierarchy under the indicated root
//               node.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
convert_hierarchy(EggGroupNode *egg_root) {
  MStatus status;

  MItDag dag_iterator(MItDag::kDepthFirst, MFn::kTransform, &status);
  if (!status) {
    status.perror("MItDag constructor");
    return false;
  }

  if (_from_selection) {
    // Get only the selected geometry.
    MSelectionList selection;
    status = MGlobal::getActiveSelectionList(selection);
    if (!status) {
      status.perror("MGlobal::getActiveSelectionList");
      return false;
    }

    // Get the selected geometry only if the selection is nonempty;
    // otherwise, get the whole scene anyway.
    if (!selection.isEmpty()) {
      bool all_ok = true;
      unsigned int length = selection.length();
      for (unsigned int i = 0; i < length; i++) {
        MDagPath root_path;
        status = selection.getDagPath(i, root_path);
        if (!status) {
          status.perror("MSelectionList::getDagPath");
        } else {
          // Now traverse through the selected dag path and all nested
          // dag paths.
          dag_iterator.reset(root_path);
          while (!dag_iterator.isDone()) {
            MDagPath dag_path;
            status = dag_iterator.getPath(dag_path);
            if (!status) {
              status.perror("MItDag::getPath");
            } else {
              if (!process_model_node(dag_path, egg_root)) {
                all_ok = false;
              }
            }
            
            dag_iterator.next();
          }
        }
      }
      return all_ok;

    } else {
      mayaegg_cat.info()
        << "Selection list is empty.\n";
      // Fall through.
    }
  }

  // Get the entire Maya scene.
    
  // This while loop walks through the entire Maya hierarchy, one
  // node at a time.  Maya's MItDag object automatically performs a
  // depth-first traversal of its scene graph.
  
  bool all_ok = true;
  while (!dag_iterator.isDone()) {
    MDagPath dag_path;
    status = dag_iterator.getPath(dag_path);
    if (!status) {
      status.perror("MItDag::getPath");
    } else {
      if (!process_model_node(dag_path, egg_root)) {
        all_ok = false;
      }
    }
    
    dag_iterator.next();
  }
  
  return all_ok;
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
process_model_node(const MDagPath &dag_path, EggGroupNode *egg_root) {
  MStatus status;
  MFnDagNode dag_node(dag_path, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return false;
  }

  if (mayaegg_cat.is_debug()) {
    mayaegg_cat.debug()
      << dag_path.fullPathName() << ": " << dag_node.typeName();

    if (MAnimUtil::isAnimated(dag_path)) {
      mayaegg_cat.debug(false)
        << " (animated)";
    }

    mayaegg_cat.debug(false) << "\n";
  }

  if (dag_node.inUnderWorld()) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring underworld node " << dag_path.fullPathName() << "\n";
    }

  } else if (dag_node.isIntermediateObject()) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring intermediate object " << dag_path.fullPathName() << "\n";
    }

  } else if (dag_path.hasFn(MFn::kCamera)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring camera node " << dag_path.fullPathName() << "\n";
    }

  } else if (dag_path.hasFn(MFn::kLight)) {
    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Ignoring light node " << dag_path.fullPathName() << "\n";
    }

  } else if (dag_path.hasFn(MFn::kJoint)) {
    // A joint.

    // Don't bother with joints unless we're getting an animatable
    // model.
    if (_animation_convert == AC_model) {
      EggGroup *egg_group = get_egg_group(dag_path, egg_root);

      if (egg_group != (EggGroup *)NULL) {
        egg_group->set_group_type(EggGroup::GT_joint);
        get_transform(dag_path, egg_group);
      }
    }

  } else if (dag_path.hasFn(MFn::kNurbsSurface)) {
    EggGroup *egg_group = get_egg_group(dag_path, egg_root);

    if (egg_group == (EggGroup *)NULL) {
      mayaegg_cat.error()
        << "Cannot determine group node.\n";
      return false;

    } else {
      if (_animation_convert != AC_model) {
        get_transform(dag_path, egg_group);
      }

      MFnNurbsSurface surface(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << dag_path.fullPathName() << ":\n"
          << "  it appears to have a NURBS surface, but does not.\n";
      } else {
        make_nurbs_surface(dag_path, surface, egg_group, egg_root);
      }
    }

  } else if (dag_path.hasFn(MFn::kNurbsCurve)) {
    // Only convert NurbsCurves if we aren't making an animated model.
    // Animated models, as a general rule, don't want these sorts of
    // things in them.
    if (_animation_convert != AC_model) {
      EggGroup *egg_group = get_egg_group(dag_path, egg_root);
      
      if (egg_group == (EggGroup *)NULL) {
        mayaegg_cat.error()
          << "Cannot determine group node.\n";
        
      } else {
        get_transform(dag_path, egg_group);
        
        MFnNurbsCurve curve(dag_path, &status);
        if (!status) {
          mayaegg_cat.info()
            << "Error in node " << dag_path.fullPathName() << ":\n"
            << "  it appears to have a NURBS curve, but does not.\n";
        } else {
          make_nurbs_curve(dag_path, curve, egg_group, egg_root);
        }
      }
    }
      
  } else if (dag_path.hasFn(MFn::kMesh)) {
    EggGroup *egg_group = get_egg_group(dag_path, egg_root);

    if (egg_group == (EggGroup *)NULL) {
      mayaegg_cat.error()
        << "Cannot determine group node.\n";
      return false;

    } else {
      if (_animation_convert != AC_model) {
        get_transform(dag_path, egg_group);
      }

      MFnMesh mesh(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << dag_path.fullPathName() << ":\n"
          << "  it appears to have a polygon mesh, but does not.\n";
      } else {
        make_polyset(dag_path, mesh, egg_group, egg_root);
      }
    }

  } else {
    // Get the translation/rotation/scale data
    EggGroup *egg_group = get_egg_group(dag_path, egg_root);

    if (egg_group != (EggGroup *)NULL) {
      if (_animation_convert != AC_model) {
        get_transform(dag_path, egg_group);
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::process_chan_node
//       Access: Private
//  Description: Similar to process_model_node(), but this code path
//               is followed only when we are building a table of
//               animation data (AC_chan).  It just builds up the
//               EggTable hierarchy according to the joint table.
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
process_chan_node(const MDagPath &dag_path, EggGroupNode *egg_root) {
  MStatus status;
  MFnDagNode dag_node(dag_path, &status);
  if (!status) {
    status.perror("MFnDagNode constructor");
    return false;
  }

  if (dag_path.hasFn(MFn::kJoint)) {
    // A joint.

    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << dag_path.fullPathName() << ": " << dag_node.typeName();
      
      if (MAnimUtil::isAnimated(dag_path)) {
        mayaegg_cat.debug(false)
          << " (animated)";
      }
      
      mayaegg_cat.debug(false) << "\n";
    }

    get_egg_table(dag_path, egg_root);
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
  if (_ignore_transforms) {
    return;
  }

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
                   EggGroup *egg_group, EggGroupNode *egg_root) {
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
    make_polyset(polyset_path, polyset_fn, egg_group, egg_root, shader);

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

  for (i = 0; i < egg_nurbs->get_num_cvs(); i++) {
    int ui = egg_nurbs->get_u_index(i);
    int vi = egg_nurbs->get_v_index(i);

    double v[4];
    MStatus status = cv_array[v_cvs * ui + vi].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      vert.set_pos(LPoint4d(v[0], v[1], v[2], v[3]));
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
                 EggGroup *egg_group, EggGroupNode *) {
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

  for (i = 0; i < egg_curve->get_num_cvs(); i++) {
    double v[4];
    MStatus status = cv_array[i].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex vert;
      vert.set_pos(LPoint4d(v[0], v[1], v[2], v[3]));
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
             EggGroup *egg_group, EggGroupNode *egg_root,
             MayaShader *default_shader) {
  MStatus status;
  string name = mesh.name().asChar();

  MObject mesh_object = mesh.object();
  bool double_sided = false;
  get_bool_attribute(mesh_object, "doubleSided", double_sided);

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

    // Since a texture completely replaces a polygon or vertex color,
    // we need to know up front whether we have a texture.
    bool has_texture = false;
    if (shader != (MayaShader *)NULL) {
      has_texture = shader->_has_texture;
    }

    // Get the vertices for the polygon.
    long num_verts = pi.polygonVertexCount();
    LPoint3d ref_p3d;
    for (long i = 0; i < num_verts; i++) {
      EggVertex vert;

      MPoint p = pi.point(i, MSpace::kWorld);
      LPoint3d p3d(p[0], p[1], p[2]);
      if (i == 0) {
        // Save the first vertex of the polygon as a reference point
        // for sealing up seams that might be introduced by a UV
        // projection, so we can ensure that all the vertices are
        // projected into the same quadrant.
        ref_p3d = p3d;
      }

      vert.set_pos(p3d);

      MVector n;
      status = pi.getNormal(i, n, MSpace::kWorld);
      if (!status) {
        status.perror("MItMeshPolygon::getNormal");
      } else {
        vert.set_normal(LVector3d(n[0], n[1], n[2]));
      }

      if (shader != (MayaShader *)NULL && shader->has_projection()) {
        // If the shader has a projection, use it instead of the
        // polygon's built-in UV's.
        vert.set_uv(shader->project_uv(p3d, ref_p3d));

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

      if (pi.hasColor() && !has_texture) {
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
      get_vertex_weights(dag_path, mesh, egg_root, joints, weights);
  }

  if (got_weights) {
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
//     Function: MayaToEggConverter::get_vertex_weights
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
bool MayaToEggConverter::
get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                   EggGroupNode *egg_root,
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
          EggGroup *joint = get_egg_group(joint_dag_path, egg_root);
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
//     Function: MayaToEggConverter::get_egg_group
//       Access: Private
//  Description: Returns the EggGroup corresponding to the indicated
//               fully-qualified Maya path name.  If there is not
//               already an EggGroup corresponding to this Maya path,
//               creates one and returns it.
//
//               In this way we generate a unique EggGroup for each
//               Maya node we care about about, and also preserve the
//               Maya hierarchy sensibly.
////////////////////////////////////////////////////////////////////
EggGroup *MayaToEggConverter::
get_egg_group(const MDagPath &dag_path, EggGroupNode *egg_root) {
  return r_get_egg_group(dag_path.fullPathName().asChar(), dag_path, egg_root);
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::r_get_egg_group
//       Access: Private
//  Description: The recursive implementation of get_egg_group().
////////////////////////////////////////////////////////////////////
EggGroup *MayaToEggConverter::
r_get_egg_group(const string &name, const MDagPath &dag_path,
                EggGroupNode *egg_root) {
  // If we have already encountered this pathname, return the
  // corresponding EggGroup immediately.
  Groups::const_iterator gi = _groups.find(name);
  if (gi != _groups.end()) {
    return (*gi).second;
  }

  // Otherwise, we have to create it.  Do this recursively, so we
  // create each node along the path.
  EggGroup *egg_group;

  if (name.empty()) {
    // This is the top.
    egg_group = (EggGroup *)NULL;

  } else {
    // Maya uses vertical bars to separate path components.  Remove
    // everything from the rightmost bar on; this will give us the
    // parent's path name.
    size_t bar = name.rfind("|");
    string parent_name, local_name;
    if (bar != string::npos) {
      parent_name = name.substr(0, bar);
      local_name = name.substr(bar + 1);
    } else {
      local_name = name;
    }

    EggGroup *parent_egg_group = 
      r_get_egg_group(parent_name, dag_path, egg_root);
    egg_group = new EggGroup(local_name);

    if (parent_egg_group != (EggGroup *)NULL) {
      parent_egg_group->add_child(egg_group);
    } else {
      egg_root->add_child(egg_group);
    }

    // Check for an object type setting, from Oliver's plug-in.
    MObject dag_object = dag_path.node();
    string object_type;
    if (get_enum_attribute(dag_object, "eggObjectTypes", object_type)) {
      egg_group->add_object_type(object_type);
    }
    if (get_enum_attribute(dag_object, "eggObjectTypes1", object_type)) {
      egg_group->add_object_type(object_type);
    }
    if (get_enum_attribute(dag_object, "eggObjectTypes2", object_type)) {
      egg_group->add_object_type(object_type);
    }
    if (get_enum_attribute(dag_object, "eggObjectTypes3", object_type)) {
      egg_group->add_object_type(object_type);
    }
  }

  _groups.insert(Groups::value_type(name, egg_group));
  return egg_group;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaToEggConverter::get_egg_table
//       Access: Private
//  Description: Returns the EggTable corresponding to the indicated
//               fully-qualified Maya path name.  This is similar to
//               get_egg_group(), but this variant is used only when
//               we are building a channel file, which is just a
//               hierarchical collection of animation tables.
////////////////////////////////////////////////////////////////////
MayaToEggConverter::JointAnim *MayaToEggConverter::
get_egg_table(const MDagPath &dag_path, EggGroupNode *egg_root) {
  string name = dag_path.fullPathName().asChar();

  // If we have already encountered this pathname, return the
  // corresponding EggTable immediately.
  Tables::const_iterator ti = _tables.find(name);
  if (ti != _tables.end()) {
    return (*ti).second;
  }

  // Otherwise, we have to create it.
  JointAnim *joint_anim;

  if (name.empty()) {
    // This is the top.
    joint_anim = (JointAnim *)NULL;

  } else {
    // Maya uses vertical bars to separate path components.  Remove
    // everything from the rightmost bar on; this will give us the
    // parent's path name.
    size_t bar = name.rfind("|");
    string parent_name, local_name;
    if (bar != string::npos) {
      parent_name = name.substr(0, bar);
      local_name = name.substr(bar + 1);
    } else {
      local_name = name;
    }

    // Look only one level up for the parent.  If it hasn't been
    // defined yet, don't define it; instead, just start from the
    // root.
    JointAnim *parent_joint_anim = NULL;
    if (!parent_name.empty()) {
      ti = _tables.find(parent_name);
      if (ti != _tables.end()) {
        parent_joint_anim = (*ti).second;
      }
    }

    joint_anim = new JointAnim;
    joint_anim->_dag_path = dag_path;
    joint_anim->_table = new EggTable(local_name);
    joint_anim->_anim = new EggXfmSAnim("xform", _egg_data->get_coordinate_system());
    joint_anim->_table->add_child(joint_anim->_anim);

    if (parent_joint_anim != (JointAnim *)NULL) {
      parent_joint_anim->_table->add_child(joint_anim->_table);
    } else {
      egg_root->add_child(joint_anim->_table);
    }
  }

  _tables.insert(Tables::value_type(name, joint_anim));
  return joint_anim;
}

////////////////////////////////////////////////////////////////////
//     Function: MayaShader::set_shader_attributes
//       Access: Public
//  Description: Applies the known shader attributes to the indicated
//               egg primitive.
////////////////////////////////////////////////////////////////////
void MayaToEggConverter::
set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader) {
  // In Maya, a polygon is either textured or colored.  The texture,
  // if present, replaces the color.

  if (shader._has_texture) {
    Filename filename = Filename::from_os_specific(shader._texture);
    Filename fullpath = 
      _path_replace->match_path(filename, get_texture_path());
    EggTexture tex(shader._name, _path_replace->store_path(fullpath));
    tex.set_fullpath(fullpath);

    tex.set_wrap_u(shader._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp);
    tex.set_wrap_v(shader._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp);
 
    // Let's mipmap all textures by default.
    tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
    tex.set_magfilter(EggTexture::FT_linear);

    LMatrix3d mat = shader.compute_texture_matrix();
    if (!mat.almost_equal(LMatrix3d::ident_mat())) {
      tex.set_transform(mat);
    }

    EggTexture *new_tex =
      _textures.create_unique_texture(tex, ~EggTexture::E_tref_name);

    primitive.set_texture(new_tex);

  } else if (shader._has_color) {
    primitive.set_color(Colorf(shader._color[0], shader._color[1], 
                               shader._color[2], 1.0f));
  }
}
