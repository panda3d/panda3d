/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file mayaToEggConverter.cxx
 * @author drose
 * @date 1999-11-10
 * Modified 19Mar10 by ETC PandaSE team
 *   Added set_vertex_color_modern to fix Phong shader bug; also see
 *   header comment for mayaToEgg.cxx for more details
 */

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
#include "eggSAnimData.h"
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
#include <maya/MFnPlugin.h>
#include <maya/MItDag.h>
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
#include <maya/MFnWeightGeometryFilter.h>
#include <maya/MFnIkJoint.h>
#include <maya/MFnSingleIndexedComponent.h>
#include <maya/MFnDoubleIndexedComponent.h>
#include <maya/MFnBlendShapeDeformer.h>
#include <maya/MItDependencyGraph.h>
#include <maya/MDagPathArray.h>
#include <maya/MSelectionList.h>
#include "post_maya_include.h"

using std::endl;
using std::string;


/**
 *
 */
MayaToEggConverter::
MayaToEggConverter(const string &program_name) :
  _program_name(program_name),
  _tree(this)
{
  // Make sure the library is properly initialized.
  init_libmayaegg();

  _from_selection = false;

  _polygon_output = false;
  _polygon_tolerance = 0.01;
  _respect_maya_double_sided = maya_default_double_sided;
  _always_show_vertex_color = maya_default_vertex_color;
  _keep_all_uvsets = false;
  _round_uvs = false;
  _legacy_shader = false;
  _convert_cameras = false;
  _convert_lights = false;

  _transform_type = TT_model;
}

/**
 *
 */
MayaToEggConverter::
MayaToEggConverter(const MayaToEggConverter &copy) :
  _program_name(copy._program_name),
  _from_selection(copy._from_selection),
  _subsets(copy._subsets),
  _subroots(copy._subroots),
  _excludes(copy._excludes),
  _ignore_sliders(copy._ignore_sliders),
  _force_joints(copy._force_joints),
  _tree(this),
  _maya(copy._maya),
  _polygon_output(copy._polygon_output),
  _polygon_tolerance(copy._polygon_tolerance),
  _respect_maya_double_sided(copy._respect_maya_double_sided),
  _always_show_vertex_color(copy._always_show_vertex_color),
  _keep_all_uvsets(copy._keep_all_uvsets),
  _convert_cameras(copy._convert_cameras),
  _convert_lights(copy._convert_lights),
  _round_uvs(copy._round_uvs),
  _legacy_shader(copy._legacy_shader),
  _transform_type(copy._transform_type)
{
}

/**
 *
 */
MayaToEggConverter::
~MayaToEggConverter() {
  close_api();
}

/**
 * Allocates and returns a new copy of the converter.
 */
SomethingToEggConverter *MayaToEggConverter::
make_copy() {
  return new MayaToEggConverter(*this);
}

/**
 * Returns the English name of the file type this converter supports.
 */
string MayaToEggConverter::
get_name() const {
  return "Maya";
}

/**
 * Returns the common extension of the file type this converter supports.
 */
string MayaToEggConverter::
get_extension() const {
  return "mb";
}

/**
 * Returns a space-separated list of extension, in addition to the one
 * returned by get_extension(), that are recognized by this converter.
 */
string MayaToEggConverter::
get_additional_extensions() const {
  return "ma";
}

/**
 * Handles the reading of the input file and converting it to egg.  Returns
 * true if successful, false otherwise.
 *
 * This is designed to be as generic as possible, generally in support of run-
 * time loading.  Also see convert_maya().
 */
bool MayaToEggConverter::
convert_file(const Filename &filename) {
  if (!open_api()) {
    mayaegg_cat.error()
      << "Maya is not available.\n";
    return false;
  }

  // We must ensure our Maya pointers are cleared before we reset the Maya
  // scene, because resetting the Maya scene will invalidate all the Maya
  // pointers we are holding and cause a crash if we try to free them later.
  clear();

  if (!_maya->read(filename)) {
    mayaegg_cat.error()
      << "Unable to read " << filename << "\n";
    return false;
  }

  if (_character_name.empty()) {
    _character_name = filename.get_basename_wo_extension();
  }

  return convert_maya();
}

/**
 * Empties the list of subroot nodes added via add_subroot().  The entire file
 * will once again be converted.
 */
void MayaToEggConverter::
clear_subroots() {
  _subroots.clear();
}

/**
 * Adds a name pattern to the list of subroot nodes.  If the list of subroot
 * nodes is not empty, then only a subroot of the nodes in the maya file will
 * be converted: those whose names match one of the patterns given on this
 * list.
 */
void MayaToEggConverter::
add_subroot(const GlobPattern &glob) {
  _subroots.push_back(glob);
}

/**
 * Empties the list of subset nodes added via add_subset().  The entire file
 * will once again be converted.
 */
void MayaToEggConverter::
clear_subsets() {
  _subsets.clear();
}

/**
 * Adds a name pattern to the list of subset nodes.  If the list of subset
 * nodes is not empty, then only a subset of the nodes in the maya file will
 * be converted: those whose names match one of the patterns given on this
 * list.
 */
void MayaToEggConverter::
add_subset(const GlobPattern &glob) {
  _subsets.push_back(glob);
}

/**
 * Empties the list of excluded nodes added via add_exclude().
 */
void MayaToEggConverter::
clear_excludes() {
  _excludes.clear();
}

/**
 * Adds a name pattern to the list of excluded nodes.
 */
void MayaToEggConverter::
add_exclude(const GlobPattern &glob) {
  _excludes.push_back(glob);
}

/**
 * Empties the list of ignore_sliders added via add_ignore_slider().  No
 * sliders will be ignored.
 */
void MayaToEggConverter::
clear_ignore_sliders() {
  _ignore_sliders.clear();
}

/**
 * Adds a name pattern to the list of ignore_sliders.  Any slider (blend shape
 * deformer) that matches a name on the list will not be converted or
 * otherwise molested by the converter.  This is occasionally necessary to
 * filter out automatically-created sliders that are not intended to be used
 * directly, but instead have an indirect effect on other sliders.
 */
void MayaToEggConverter::
add_ignore_slider(const GlobPattern &glob) {
  _ignore_sliders.push_back(glob);
}

/**
 * Returns true if the indicated name is on the list of sliders to ignore,
 * false otherwise.
 */
bool MayaToEggConverter::
ignore_slider(const string &name) const {
  Globs::const_iterator gi;
  for (gi = _ignore_sliders.begin(); gi != _ignore_sliders.end(); ++gi) {
    if ((*gi).matches(name)) {
      return true;
    }
  }

  return false;
}

/**
 * Empties the list of force_joints added via add_force_joint().  No joints
 * will be forced.
 */
void MayaToEggConverter::
clear_force_joints() {
  _force_joints.clear();
}

/**
 * Adds a name pattern to the list of force_joints.
 *
 * Any DAG node that matches a name on the list will be treated as if it were
 * a joint during the conversion process; it will receive animation and
 * position information.  Normally, a true Maya joint, as well as any DAG
 * nodes whose transforms are animated, will automatically be flagged as a
 * Panda joint.
 */
void MayaToEggConverter::
add_force_joint(const GlobPattern &glob) {
  _force_joints.push_back(glob);
}

/**
 * Returns true if the indicated name is on the list of DAG nodes to treat as
 * a joint, false otherwise.
 */
bool MayaToEggConverter::
force_joint(const string &name) const {
  Globs::const_iterator gi;
  for (gi = _force_joints.begin(); gi != _force_joints.end(); ++gi) {
    if ((*gi).matches(name)) {
      return true;
    }
  }

  return false;
}

/**
 * Sets the flag that indicates whether the currently selected Maya geometry
 * will be converted.  If this is true, and the selection is nonempty, then
 * only the selected geometry will be converted.  If this is false, the entire
 * file will be converted.
 */
void MayaToEggConverter::
set_from_selection(bool from_selection) {
  _from_selection = from_selection;
}

/**
 * This may be called after convert_file() has been called and returned true,
 * indicating a successful conversion.  It will return the distance units
 * represented by the converted egg file, if known, or DU_invalid if not
 * known.
 */
DistanceUnit MayaToEggConverter::
get_input_units() {
  return _maya->get_units();
}

/**
 * Fills up the egg_data structure according to the global maya model data.
 * Returns true if successful, false if there is an error.
 */
bool MayaToEggConverter::
convert_maya() {
  clear();
  clear_error();

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
    // end_frame = MAnimControl::animationEndTime().value(); masad: we could
    // use this
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

  frame_inc = frame_inc * input_frame_rate / output_frame_rate;

  bool all_ok = _tree.build_hierarchy();

  if (all_ok) {
    if (!_subroots.empty()) {
      Globs::const_iterator gi;
      for (gi = _subroots.begin(); gi != _subroots.end(); ++gi) {
        if (!_tree.tag_joint_named(*gi)) {
          mayaegg_cat.info()
            << "No node matching " << *gi << " found.\n";
        }
      }

    } else {
      // This call makes every node a potential joint; but it does not
      // necessarily force nodes to be joints.
      _tree.tag_joint_all();
    }
  }

  if (all_ok) {
    if (_from_selection) {
      all_ok = _tree.tag_selected();
    } else if (!_subsets.empty()) {
      Globs::const_iterator gi;
      for (gi = _subsets.begin(); gi != _subsets.end(); ++gi) {
        if (!_tree.tag_named(*gi)) {
          mayaegg_cat.info()
            << "No node matching " << *gi << " found.\n";
        }
      }

    } else {
      _tree.tag_all();
    }
  }

  if (all_ok) {
    if (!_excludes.empty()) {
      Globs::const_iterator gi;
      for (gi = _excludes.begin(); gi != _excludes.end(); ++gi) {
        if (!_tree.untag_named(*gi)) {
          mayaegg_cat.info()
            << "No node matching " << *gi << " found.\n";
        }
      }
    }
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
      mayaegg_cat.info() << "ac_none" << endl;
      all_ok = convert_hierarchy(get_egg_data());
      break;

    case AC_flip:
    case AC_strobe:
      // flip or strobe: get out a series of static models, one per frame,
      // under a sequence node for AC_flip.
      all_ok = convert_flip(start_frame, end_frame, frame_inc,
                            output_frame_rate);
      break;

    case AC_model:
      // model: get out an animatable model with joints and vertex membership.
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

    case AC_invalid:
      break;
    };

    reparent_decals(get_egg_data());
  }

  if (had_error()) {
    all_ok = false;
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

/**
 * Attempts to open the Maya API if it was not already open, and returns true
 * if successful, or false if there is an error.
 */
bool MayaToEggConverter::
open_api(bool revert_directory) {

  if (_maya == nullptr || !_maya->is_valid()) {
    // maya to egg converter only needs a read license.  only egg2maya need
    // write lisences.
    _maya = MayaApi::open_api(_program_name, true, revert_directory);
  }
  return _maya->is_valid();
}

/**
 * Closes the Maya API, if it was previously opened.  Caution!  Maya appears
 * to call exit() when its API is closed.
 */
void MayaToEggConverter::
close_api() {
  // We have to clear the shaders, at least, before we release the Maya API.
  clear();
  _maya.clear();
}

/**
 * Frees all of the Maya pointers kept within this object, in preparation for
 * loading a new scene or releasing the Maya API.
 */
void MayaToEggConverter::
clear() {
  _tree.clear();
  _textures.clear();
  _shaders.clear();
}

/**
 * Converts the animation as a series of models that cycle (flip) from one to
 * the next at the appropriate frame rate.  This is the most likely to convert
 * precisely (since we ask Maya to tell us the vertex position each time) but
 * it is the most wasteful in terms of memory utilization (since a complete of
 * the model is stored for each frame).
 */
bool MayaToEggConverter::
convert_flip(double start_frame, double end_frame, double frame_inc,
             double output_frame_rate) {
  bool all_ok = true;

  EggGroup *sequence_node = new EggGroup(_character_name);
  get_egg_data()->add_child(sequence_node);
  if (_animation_convert == AC_flip) {
    sequence_node->set_switch_flag(true);
    sequence_node->set_switch_fps(output_frame_rate);
  }

  MTime frame(start_frame, MTime::uiUnit());
  MTime frame_stop(end_frame, MTime::uiUnit());
  while (frame <= frame_stop) {
    mayaegg_cat.info(false)
      << "frame " << frame.value() << "\n";
    std::ostringstream name_strm;
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

/**
 * Converts the file as an animatable character model, with joints and vertex
 * membership.
 */
bool MayaToEggConverter::
convert_char_model() {
  if (has_neutral_frame()) {
    MTime frame(get_neutral_frame(), MTime::uiUnit());
    mayaegg_cat.info(false)
      << "neutral frame " << frame.value() << "\n";
    MGlobal::viewFrame(frame);
  }

  // It's also important for us to reset all the blend shape sliders to 0
  // before we get out the model.  Otherwise, the model we convert will have
  // the current positions of the sliders baked in.
  _tree.reset_sliders();

  EggGroup *char_node = new EggGroup(_character_name);
  get_egg_data()->add_child(char_node);
  char_node->set_dart_type(EggGroup::DT_default);

  return convert_hierarchy(char_node);
}

/**
 * Converts the animation as a series of tables to apply to the character
 * model, as retrieved earlier via AC_model.
 */
bool MayaToEggConverter::
convert_char_chan(double start_frame, double end_frame, double frame_inc,
                  double output_frame_rate) {
  // MStatus status;

  EggTable *root_table_node = new EggTable();
  get_egg_data()->add_child(root_table_node);
  EggTable *bundle_node = new EggTable(_character_name);
  bundle_node->set_table_type(EggTable::TT_bundle);
  root_table_node->add_child(bundle_node);
  EggTable *skeleton_node = new EggTable("<skeleton>");
  bundle_node->add_child(skeleton_node);
  EggTable *morph_node = new EggTable("morph");
  bundle_node->add_child(morph_node);

  // Set the frame rate before we start asking for anim tables to be created.
  _tree._fps = output_frame_rate;
  _tree.clear_egg(get_egg_data(), nullptr, skeleton_node, morph_node);

  // Now we can get the animation data by walking through all of the frames,
  // one at a time, and getting the joint angles at each frame.

  // This is just a temporary EggGroup to receive the transform for each joint
  // each frame.
  PT(EggGroup) tgroup = new EggGroup;

  int num_nodes = _tree.get_num_nodes();
  int num_sliders = _tree.get_num_blend_descs();
  int i;

  MTime frame(start_frame, MTime::uiUnit());
  MTime frame_stop(end_frame, MTime::uiUnit());
  while (frame <= frame_stop) {
    if (mayaegg_cat.is_spam()) {
      mayaegg_cat.spam(false)
        << "frame " << frame.value() << "\n";
    } else {
      // We have to write to cerr instead of mayaegg_cat to allow flushing
      // without writing a newline.
      std::cerr << "." << std::flush;
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
        if (!anim->add_data(tgroup->get_transform3d())) {
          mayaegg_cat.error()
            << "Invalid transform on " << node_desc->get_name()
            << " frame " << frame.value() << ".\n";
        }
      }
    }

    for (i = 0; i < num_sliders; i++) {
      MayaBlendDesc *blend_desc = _tree.get_blend_desc(i);
      if (mayaegg_cat.is_spam()) {
        mayaegg_cat.spam()
          << "slider " << blend_desc->get_name() << "\n";
      }
      EggSAnimData *anim = _tree.get_egg_slider(blend_desc);
      anim->add_data(blend_desc->get_slider());
    }

    frame += frame_inc;
  }

  // Now optimize all of the tables we just filled up, for no real good
  // reason, except that it makes the resulting egg file a little easier to
  // read.
  for (i = 0; i < num_nodes; i++) {
    MayaNodeDesc *node_desc = _tree.get_node(i);
    if (node_desc->is_joint()) {
      _tree.get_egg_anim(node_desc)->optimize();
    }
  }

  for (i = 0; i < num_sliders; i++) {
    MayaBlendDesc *blend_desc = _tree.get_blend_desc(i);
    EggSAnimData *anim = _tree.get_egg_slider(blend_desc);
    anim->optimize();
  }

  mayaegg_cat.info(false)
    << "\n";

  return true;
}

/**
 * Generates egg structures for each node in the Maya hierarchy.
 */
bool MayaToEggConverter::
convert_hierarchy(EggGroupNode *egg_root) {
  int num_nodes = _tree.get_num_nodes();

  if (_round_uvs) {
    mayaegg_cat.info() << "will round up uv coordinates" << endl;
  }

  if (_keep_all_uvsets) {
    mayaegg_cat.info() << "will keep_all_uvsets" << endl;
  }
  if (_polygon_output) {
    mayaegg_cat.info() << "will convert NURBS to polys" << endl;
  }
  if (_convert_cameras) {
    mayaegg_cat.info() << "will convert camera nodes to locators" << endl;
  }
  if (_convert_lights) {
    mayaegg_cat.info() << "will convert light nodes to locators" << endl;
  }
  // give some feedback about whether special options are on
  if (_legacy_shader) {
    mayaegg_cat.info() << "will disable modern Phong shader path. using legacy" << endl;
  }
  _tree.clear_egg(get_egg_data(), egg_root, nullptr, nullptr);
  for (int i = 0; i < num_nodes; i++) {
    MayaNodeDesc *node = _tree.get_node(i);
    if (!process_model_node(node)) {
      return false;
    }
  }
  return true;
}

/**
 * Converts the indicated Maya node (given a MDagPath, similar in concept to
 * Panda's NodePath) to the corresponding Egg structure.  Returns true if
 * successful, false if an error was encountered.
 */
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
    mayaegg_cat.error() << dag_path.fullPathName().asChar() << "\n";
    return false;
  }

  MObject node = dag_path.transform(&status);
  if (!status) {
    status.perror("dag_path.transform()");
    return false;
  }

  string path = dag_path.fullPathName().asChar();

  if (mayaegg_cat.is_debug()) {
    mayaegg_cat.debug()
      << path << ": " << dag_node.typeName().asChar();

    if (MAnimUtil::isAnimated(dag_path)) {
      mayaegg_cat.debug(false)
        << " (animated)";
    }

    mayaegg_cat.debug(false) << "\n";
  }

  if (dag_node.inUnderWorld()) {
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
    if (_convert_cameras) {
      MFnCamera camera (dag_path, &status);
      if ( !status ) {
        status.perror("MFnCamera constructor");
        return false;
      }

      // Extract some interesting Camera data
      if (mayaegg_cat.is_spam()) {
        MPoint eyePoint = camera.eyePoint(MSpace::kWorld);
        MVector upDirection = camera.upDirection(MSpace::kWorld);
        MVector viewDirection = camera.viewDirection(MSpace::kWorld);
        mayaegg_cat.spam() << "  eyePoint: " << eyePoint.x << " "
                           << eyePoint.y << " " << eyePoint.z << endl;
        mayaegg_cat.spam() << "  upDirection: " << upDirection.x << " "
                           << upDirection.y << " " << upDirection.z << endl;
        mayaegg_cat.spam() << "  viewDirection: " << viewDirection.x << " "
                           << viewDirection.y << " " << viewDirection.z << endl;
        mayaegg_cat.spam() << "  aspectRatio: " << camera.aspectRatio() << endl;
        mayaegg_cat.spam() << "  horizontalFilmAperture: "
                           << camera.horizontalFilmAperture() << endl;
        mayaegg_cat.spam() << "  verticalFilmAperture: "
                           << camera.verticalFilmAperture() << endl;
      }

      EggGroup *egg_group = _tree.get_egg_group(node_desc);

      if (mayaegg_cat.is_debug()) {
        mayaegg_cat.warning()
          << "Saving camera nodes as a locator: " << path << "\n";
      }

      if (node_desc->is_tagged()) {
        // Presumably, the camera's position has some meaning to the end-user,
        // so we will implicitly tag it with the DCS flag so it won't get
        // flattened out.
        if (_animation_convert != AC_model) {
          // For now, don't set the DCS flag on cameras within character
          // models, since egg-optchar doesn't understand this.  Perhaps
          // there's no reason to ever change this, since cameras within
          // character models may not be meaningful.
          egg_group->set_dcs_type(EggGroup::DC_net);
        }
        get_transform(node_desc, dag_path, egg_group);
        make_camera_locator(dag_path, dag_node, egg_group);
      } else {
        if (mayaegg_cat.is_debug()) {
          mayaegg_cat.debug()
            << "Ignoring camera node " << path
            << "\n";
        }
      }
    }

  } else if (dag_path.hasFn(MFn::kLight)) {
    if (_convert_lights) {
      MFnLight light (dag_path, &status);
      if ( !status ) {
        status.perror("MFnLight constructor");
        return false;
      }

      EggGroup *egg_group = _tree.get_egg_group(node_desc);

      if (mayaegg_cat.is_debug()) {
        mayaegg_cat.warning() << "Saving light node as a locator: " << path << endl;
      }

      if (node_desc->is_tagged()) {
        // Presumably, the lighht's position has some meaning to the end-user,
        // so we will implicitly tag it with the DCS flag so it won't get
        // flattened out.
        if (_animation_convert != AC_model) {
          // For now, don't set the DCS flag on lights within character
          // models, since egg-optchar doesn't understand this.  Perhaps
          // there's no reason to ever change this, since lights within
          // character models may not be meaningful.
          egg_group->set_dcs_type(EggGroup::DC_net);
        }
        get_transform(node_desc, dag_path, egg_group);
        make_light_locator(dag_path, dag_node, egg_group);
      } else {
        if (mayaegg_cat.is_debug()) {
          mayaegg_cat.debug()
            << "Ignoring light node " << path
            << "\n";
          }
        }
      }

    MFnLight light (dag_path, &status);
    if ( !status ) {
      status.perror("MFnLight constructor");
      mayaegg_cat.error() << "light extraction failed" << endl;
      return false;
    }

    if (mayaegg_cat.is_info()) {
      MString name = dag_path.partialPathName();
      mayaegg_cat.info() << "-- Light found -- tranlations in cm, rotations in rads\n";
      mayaegg_cat.info() << "\"" << name.asChar() << "\" : \n";
    }

    // Get the translationrotationscale data
    MObject transformNode = dag_path.transform(&status);
    // This node has no transform - i.e., it's the world node
    if (!status && status.statusCode () == MStatus::kInvalidParameter)
      return false;
    MFnDagNode  transform (transformNode, &status);
    if (!status) {
      status.perror("MFnDagNode constructor");
      return false;
    }
    MTransformationMatrix matrix (transform.transformationMatrix());
    MVector tl = matrix.translation(MSpace::kWorld);
    // Stop rediculously small values like -4.43287e-013
    if (tl.x < 0.0001) {
      tl.x = 0;
    }
    if (tl.y < 0.0001) {
      tl.y = 0;
    }
    if (tl.z < 0.0001) {
      tl.z = 0;
    }
    // We swap Y and Z in the next few bits cuz Panda is Z-up by default and
    // Maya is Y-up
    mayaegg_cat.info() << "  \"translation\" : (" << tl.x << ", " << tl.z << ", " << tl.y << ")"
         << endl;
    double threeDoubles[3];
    MTransformationMatrix::RotationOrder  rOrder;

    matrix.getRotation (threeDoubles, rOrder, MSpace::kWorld);
    mayaegg_cat.info() << "  \"rotation\": ("
         << threeDoubles[0] << ", "
         << threeDoubles[2] << ", "
         << threeDoubles[1] << ")\n";
    matrix.getScale (threeDoubles, MSpace::kWorld);
    mayaegg_cat.info() << "  \"scale\" : ("
         << threeDoubles[0] << ", "
         << threeDoubles[2] << ", "
         << threeDoubles[1] << ")\n";

    // Extract some interesting Light data
    MColor color;
    color = light.color();
    mayaegg_cat.info() << "  \"color\" : ("
         << color.r << ", "
         << color.g << ", "
         << color.b << ")\n";
    color = light.shadowColor();
    mayaegg_cat.info() << "  \"intensity\" : " << light.intensity() << endl;

  } else if (dag_path.hasFn(MFn::kNurbsSurface)) {
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(node_desc, dag_path, egg_group);

    if (node_desc->is_tagged()) {
      MFnNurbsSurface surface(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << path
          << ":\n"
          << "  it appears to have a NURBS surface, but does not.\n";
      } else {
        make_nurbs_surface(node_desc, dag_path, surface, egg_group);
      }
    }
  } else if (dag_path.hasFn(MFn::kNurbsCurve)) {
    // Only convert NurbsCurves if we aren't making an animated model.
    // Animated models, as a general rule, don't want these sorts of things in
    // them.
    if (_animation_convert != AC_model) {
      EggGroup *egg_group = _tree.get_egg_group(node_desc);
      get_transform(node_desc, dag_path, egg_group);

      if (node_desc->is_tagged()) {
        MFnNurbsCurve curve(dag_path, &status);
        if (!status) {
          mayaegg_cat.info()
            << "Error in node " << path << ":\n"
            << "  it appears to have a NURBS curve, but does not.\n";
        } else {
          make_nurbs_curve(dag_path, curve, egg_group);
        }
      }
    }

  } else if (dag_path.hasFn(MFn::kMesh)) {
    if (node_desc->is_tagged()) {
      EggGroup *egg_group = _tree.get_egg_group(node_desc);
      get_transform(node_desc, dag_path, egg_group);
      MFnMesh mesh(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << path << ":\n"
          << "  it appears to have a polygon mesh, but does not.\n";
      } else {
        make_polyset(node_desc, dag_path, mesh, egg_group);
      }
    }
    /*
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(node_desc, dag_path, egg_group);

    if (node_desc->is_tagged()) {
      MFnMesh mesh(dag_path, &status);
      if (!status) {
        mayaegg_cat.info()
          << "Error in node " << path << ":\n"
          << "  it appears to have a polygon mesh, but does not.\n";
      } else {
        make_polyset(node_desc, dag_path, mesh, egg_group);
      }
    }
    */
  } else if (dag_path.hasFn(MFn::kLocator)) {
    if (_animation_convert == AC_none) {
      if (!node_desc->is_tagged()) {
        return true;
      }
    }
    EggGroup *egg_group = _tree.get_egg_group(node_desc);

    if (mayaegg_cat.is_debug()) {
      mayaegg_cat.debug()
        << "Locator at " << path << "\n";
    }

    if (node_desc->is_tagged()) {
      // Presumably, the locator's position has some meaning to the end-user,
      // so we will implicitly tag it with the DCS flag so it won't get
      // flattened out.
      if (_animation_convert != AC_model) {
        // For now, don't set the DCS flag on locators within character
        // models, since egg-optchar doesn't understand this.  Perhaps there's
        // no reason to ever change this, since locators within character
        // models may not be meaningful.
        egg_group->set_dcs_type(EggGroup::DC_net);
      }
      get_transform(node_desc, dag_path, egg_group);
      make_locator(dag_path, dag_node, egg_group);
    }

  } else {
    // Just a generic node.
    if (_animation_convert == AC_none) {
      if (!node_desc->is_tagged()) {
        return true;
      }
    }
    EggGroup *egg_group = _tree.get_egg_group(node_desc);
    get_transform(node_desc, dag_path, egg_group);
  }

  return true;
}

/**
 * Extracts the transform on the indicated Maya node, and applies it to the
 * corresponding Egg node.
 */
void MayaToEggConverter::
get_transform(MayaNodeDesc *node_desc, const MDagPath &dag_path,
              EggGroup *egg_group) {
  if (_animation_convert == AC_model) {
    // When we're getting an animated model, we only get transforms for
    // joints, and they get converted in a special way.

    if (node_desc->is_joint()) {
      if (mayaegg_cat.is_spam()) {
        mayaegg_cat.spam()
          << "gt: joint " << node_desc->get_name() << "\n";
      }
      get_joint_transform(dag_path, egg_group);
    }
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
      if (!egg_group->get_model_flag() && !egg_group->has_dcs_type()) {
        return;
      }
      break;

    case TT_dcs:
      if (!egg_group->has_dcs_type()) {
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

  // Maya has a rotate pivot, separate from its transform.  Usually we care
  // more about the rotate pivot than we do about the transform, so get the
  // rotate pivot too.
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

  // We need to convert the pivot to world coordinates.  (Maya can only tell
  // it to us in local coordinates.)
  LPoint3d p3d(pivot[0], pivot[1], pivot[2]);
  p3d = p3d * m4d;

  // Now recenter the matrix about the pivot point.
  m4d.set_row(3, p3d);

  // Convert the recentered matrix into the group's space and store it.
  m4d = m4d * egg_group->get_node_frame_inv();
  if (!m4d.almost_equal(LMatrix4d::ident_mat(), 0.0001)) {
    egg_group->add_matrix4(m4d);
  }
  return;
}

/**
 * Extracts the transform on the indicated Maya node, as appropriate for a
 * joint in an animated character, and applies it to the indicated node.  This
 * is different from get_transform() in that it does not respect the
 * _transform_type flag, and it does not consider the relative transforms
 * within the egg file.
 */
void MayaToEggConverter::
get_joint_transform(const MDagPath &dag_path, EggGroup *egg_group) {
  // First, make sure there's not a transform on the group already.
  egg_group->clear_transform();

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
    MVector t = matrix.translation(MSpace::kWorld);
    mayaegg_cat.spam()
      << "  translation: ["
      << t[0] << ", "
      << t[1] << ", "
      << t[2] << "]\n";
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
    egg_group->set_transform3d
      (LMatrix4d(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                 mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                 mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                 mat[3][0], mat[3][1], mat[3][2], mat[3][3]));
  }
}

/**
 * Converts the indicated Maya NURBS surface to a corresponding egg structure,
 * and attaches it to the indicated egg group.
 */
void MayaToEggConverter::
make_nurbs_surface(MayaNodeDesc *node_desc, const MDagPath &dag_path,
                   MFnNurbsSurface &surface, EggGroup *egg_group) {
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
  MayaShader *shader = _shaders.find_shader_for_node(surface.object(), _legacy_shader);

  if (_polygon_output) {
    // If we want polygon output only, tesselate the NURBS and output that.
    MTesselationParams params;
    params.setFormatType(MTesselationParams::kStandardFitFormat);
    params.setOutputType(MTesselationParams::kQuads);
    params.setStdFractionalTolerance(_polygon_tolerance);

    // We'll create the tesselation as a sibling of the NURBS surface.  That
    // way we inherit all of the transformations.
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
    make_polyset(node_desc, polyset_path, polyset_fn, egg_group, shader);

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

  // Also get out all the alternate blend shapes for the surface by applying
  // each morph slider one at a time.
  pvector<MPointArray> morph_cvs;
  if (_animation_convert == AC_model) {
    int num_sliders = node_desc->get_num_blend_descs();
    morph_cvs.reserve(num_sliders);
    for (int i = 0; i < num_sliders; i++) {
      MayaBlendDesc *blend_desc = node_desc->get_blend_desc(i);

      // Temporarily push the slider up to 1.0 so we can see what the surface
      // looks like at that value.
      blend_desc->set_slider(1.0);
      MPointArray cv_array;
      status = surface.getCVs(cv_array, MSpace::kWorld);
      blend_desc->set_slider(0.0);

      if (!status) {
        status.perror("MFnNurbsSurface::getCVs");
        return;
      }
      morph_cvs.push_back(cv_array);
    }
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

  MFnNurbsSurface::Form u_form = surface.formInU();
  MFnNurbsSurface::Form v_form = surface.formInV();

  int u_degree = surface.degreeU();
  int v_degree = surface.degreeV();

  int u_cvs = surface.numCVsInU();
  int v_cvs = surface.numCVsInV();

  // Maya repeats CVS at the end for a periodic surface, and doesn't count
  // them in the joint weight array, below.
  int maya_u_cvs = (u_form == MFnNurbsSurface::kPeriodic) ? u_cvs - u_degree : u_cvs;
  int maya_v_cvs = (v_form == MFnNurbsSurface::kPeriodic) ? v_cvs - v_degree : v_cvs;

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
    int maya_vi = v_cvs * ui + vi;

    double v[4];
    status = cv_array[maya_vi].get(v);
    if (!status) {
      status.perror("MPoint::get");
    } else {
      EggVertex *vert = vpool->add_vertex(new EggVertex, i);
      LPoint4d p4d(v[0], v[1], v[2], v[3]);
      p4d = p4d * vertex_frame_inv;
      vert->set_pos(p4d);

      // Now generate the morph targets for the vertex.
      if (!morph_cvs.empty()) {
        // Morph deltas are given in 3-d space, not in 4-d homogenous space.
        LPoint3d p3d(v[0] / v[3], v[1] / v[3], v[2] / v[3]);

        for (unsigned int si = 0; si < morph_cvs.size(); si++) {
          MayaBlendDesc *blend_desc = node_desc->get_blend_desc(si);
          status = morph_cvs[si][maya_vi].get(v);
          if (!status) {
            status.perror("MPoint::get");
          } else {
            LPoint3d m3d(v[0] / v[3], v[1] / v[3], v[2] / v[3]);
            LVector3d delta = m3d - p3d;
            if (!delta.almost_equal(LVector3d::zero())) {
              EggMorphVertex dxyz(blend_desc->get_name(), delta);
              vert->_dxyzs.insert(dxyz);
            }
          }
        }
      }

      egg_nurbs->add_vertex(vert);
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
                    if (egg_curve != nullptr) {
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

  // We add the NURBS to the group down here, after all of the vpools for the
  // trim curves have been added.
  egg_group->add_child(egg_nurbs);

  if (shader != nullptr) {
    set_shader_attributes(*egg_nurbs, *shader);
  }

  // Now try to find the skinning information for the surface.
  bool got_weights = false;

  pvector<EggGroup *> joints;
  MFloatArray weights;
  if (_animation_convert == AC_model) {
    got_weights =
      get_vertex_weights(dag_path, surface, joints, weights);
  }

  if (got_weights && !joints.empty()) {
    int num_joints = joints.size();
    int num_weights = (int)weights.length();
    int num_verts = num_weights / num_joints;
    // The number of weights should be an even multiple of verts * joints.
    nassertv(num_weights == num_verts * num_joints);

    for (i = 0; i < egg_nurbs->get_num_cvs(); i++) {
      int ui = egg_nurbs->get_u_index(i) % maya_u_cvs;
      int vi = egg_nurbs->get_v_index(i) % maya_v_cvs;

      int maya_vi = maya_v_cvs * ui + vi;
      nassertv(maya_vi < num_verts);
      EggVertex *vert = vpool->get_vertex(i);

      for (int ji = 0; ji < num_joints; ++ji) {
        PN_stdfloat weight = weights[maya_vi * num_joints + ji];
        if (weight != 0.0f) {
          EggGroup *joint = joints[ji];
          if (joint != nullptr) {
            joint->ref_vertex(vert, weight);
          }
        }
      }
    }
  }
}

/**
 * Converts the indicated Maya NURBS trim curve to a corresponding egg
 * structure, and returns it, or NULL if there is a problem.
 */
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
    return nullptr;
  }
  MDoubleArray knot_array;
  status = curve.getKnots(knot_array);
  if (!status) {
    status.perror("MFnNurbsCurve::getKnots");
    return nullptr;
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

/**
 * Converts the indicated Maya NURBS curve (a standalone curve, not a trim
 * curve) to a corresponding egg structure and attaches it to the indicated
 * egg group.
 */
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
  MayaShader *shader = _shaders.find_shader_for_node(curve.object(), _legacy_shader);
  if (shader != nullptr) {
    set_shader_attributes(*egg_curve, *shader);
  }
}

/**
 * given uvsets, round them up or down
 */
int MayaToEggConverter::
round(double value) {
  if (value < 0)
    return -(floor(-value + 0.5));
  // or as an alternate use: return ceil ( value - 0.5);
  else
    return   floor( value + 0.5);
}

/**
 * Converts the indicated Maya polyset to a bunch of EggPolygons and parents
 * them to the indicated egg group.
 */
void MayaToEggConverter::
make_polyset(MayaNodeDesc *node_desc, const MDagPath &dag_path,
             const MFnMesh &mesh, EggGroup *egg_group,
             MayaShader *default_shader) {
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

  // One way to convert the mesh would be to first get out all the vertices in
  // the mesh and add them into the vpool, then when we traverse the polygons
  // we would only have to index them into the vpool according to their Maya
  // vertex index.

  // Unfortunately, since Maya may store multiple normals andor colors for
  // each vertex according to which polygon it is in, that approach won't
  // necessarily work.  In egg, those split-property vertices have to become
  // separate vertices.  So instead of adding all the vertices up front, we'll
  // start with an empty vpool, and add vertices to it on the fly.

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

  // We will need to transform all vertices from world coordinate space into
  // the vertex space appropriate to this node.  Usually, this is the same
  // thing as world coordinate space, and this matrix will be identity; but if
  // the node is under an instance (particularly, for instance, a billboard)
  // then the vertex space will be different from world space.
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

  bool double_sided = maya_double_sided;
  if (!_respect_maya_double_sided) {
    // If this flag is false, we respect the maya double-sided settings only
    // if the egg "double-sided" flag is also set.
    if (!egg_double_sided) {
      double_sided = false;
    }
  }

  bool keep_all_uvsets = _keep_all_uvsets || node_desc->has_object_type("keep-all-uvsets");
  if (node_desc->has_object_type("keep-all-uvsets")) {
    mayaegg_cat.info() << "will keep_all_uvsets" << endl;
  }

  _shaders.bind_uvsets(mesh.object());

  while (!pi.isDone()) {
    EggPolygon *egg_poly = new EggPolygon;
    egg_group->add_child(egg_poly);

    egg_poly->set_bface_flag(double_sided);

    // Determine the MayaShader for this particular polygon.  There appears to
    // be two diverging paths for any Maya node with a Material (MayaShader)
    // on it This next bit kicks us out into mayaShader et al.  to pull
    // textures and everything else.
    MayaShader *shader = nullptr;
    int index = pi.index();
    nassertv(index >= 0 && index < (int)poly_shader_indices.length());
    int shader_index = poly_shader_indices[index];

    if (shader_index != -1) {
      nassertv(shader_index >= 0 && shader_index < (int)shaders.length());
      MObject engine = shaders[shader_index];
      shader =
        _shaders.find_shader_for_shading_engine(engine, _legacy_shader); //head out to the other classes
      // does this mean if we didn't find a Maya shader give it a default
      // value anyway?
    } else if (default_shader != nullptr) {
      shader = default_shader;
    }

    const MayaShaderColorDef *default_color_def = nullptr;

    // And apply the shader properties to the polygon.
    if (shader != nullptr) {
      set_shader_attributes(*egg_poly, *shader, true);
      default_color_def = shader->get_color_def();
    }

    // Should we extract the color from the vertices?  Normally, in Maya a
    // texture completely replaces the vertex color, so we should ignore the
    // vertex color if we have a texture.

    // However, this is an inconvenient property of Maya; sometimes we really
    // do want both vertex color and texture applied to the same object.  To
    // allow this, we define the special egg flag "vertex-color", which when
    // set indicates that we should respect the vertex color anyway.

    // Furthermore, if _always_show_vertex_color is true, we pretend that the
    // "vertex-color" flag is always set.
    bool ignore_vertex_color = false;
    if ( default_color_def != nullptr) {
      ignore_vertex_color = default_color_def->_has_texture && !(egg_vertex_color || _always_show_vertex_color);
    }

    LColor poly_color(1.0f, 1.0f, 1.0f, 1.0f);
    if (!ignore_vertex_color) {
      // If we're respecting the vertex color, then remove the color
      // specification from the polygon (so we can apply it to the vertices).
      poly_color = egg_poly->get_color();
      egg_poly->clear_color();
    }

    // Get the vertices for the polygon.
    long num_verts = pi.polygonVertexCount();
    long i;
    LPoint3d centroid(0.0, 0.0, 0.0);

    if (default_color_def != nullptr && default_color_def->has_projection()) {
      // If the shader has a projection, we may need to compute the polygon's
      // centroid to avoid seams at the edges.
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
      LPoint3d p3d(p[0] / p[3], p[1] / p[3], p[2] / p[3]);
      p3d = p3d * vertex_frame_inv;
      vert.set_pos(p3d);

      MVector n;
      status = pi.getNormal(i, n, MSpace::kWorld);
      if (!status) {
        status.perror("MItMeshPolygon::getNormal");
      } else {
        LNormald n3d(n[0], n[1], n[2]);
        n3d = n3d * vertex_frame_inv;
        vert.set_normal(n3d);
      }

      // Go thru all the texture references for this primitive and set uvs
      if (mayaegg_cat.is_spam()) {
        if (shader != nullptr) {
          mayaegg_cat.spam() << "shader->_color.size is " << shader->_color.size() << endl;
        }
        mayaegg_cat.spam() << "primitive->tref.size is " << egg_poly->get_num_textures() << endl;
      }
      for (size_t ti=0; ti< _shaders._uvset_names.size(); ++ti) {
        // get the eggTexture pointer
        string uvset_name(_shaders._uvset_names[ti]);
        string panda_uvset_name = uvset_name;
        if (panda_uvset_name == "map1") {
          panda_uvset_name = "default";
        }
        if (mayaegg_cat.is_spam()) {
          mayaegg_cat.spam() << "--uvset_name :" << uvset_name << endl;
        }

        // get the shader color def that matches this EggTexture Asad:
        // optimizing uvset: to discard unused uvsets.  This for loop figures
        // out which ones are unused.

        bool keep_uv = keep_all_uvsets;
        bool project_uv = false;
        LTexCoordd uv_projection;

        if (shader != nullptr) {
          for (size_t tj = 0; tj < shader->_all_maps.size(); ++tj) {
            MayaShaderColorDef *def = shader->_all_maps[tj];
            if (def->_uvset_name == uvset_name) {
              if (mayaegg_cat.is_spam()) {
                mayaegg_cat.spam() << "matched colordef idx: " << tj << endl;
              }
              keep_uv = true;
              if (def->has_projection()) {
                project_uv = true;
                uv_projection = def->project_uv(p3d, centroid);
              }
              break;
            }
          }
        }

        // if uvset is not used don't add it to the vertex
        if (!keep_uv) {
          if (mayaegg_cat.is_spam()) {
            mayaegg_cat.spam() << "discarding unused uvset " << uvset_name << endl;
          }
          continue;
        }

        if (project_uv) {
          // If the shader has a projection, use it instead of the polygon's
          // built-in UV's.
          vert.set_uv(panda_uvset_name, uv_projection);
        } else {
          // Get the UV's from the polygon.
          float2 uvs;
          MString uv_mstring(uvset_name.c_str());
          if (pi.hasUVs(uv_mstring, &status)) {
            status = pi.getUV(i, uvs, &uv_mstring);
            if (!status) {
              status.perror("MItMeshPolygon::getUV");
            } else {
              if (_round_uvs) {
                if (uvs[0] > 1.0 || uvs[0] < -1.0) {
                  // apply upto 11000th precision, but round up
                  uvs[0] = (long)(uvs[0]*1000);
                  mayaegg_cat.debug() << "before rounding uvs[0]: " << uvs[0] << endl;
                  uvs[0] = (double)(round((double)uvs[0]/10.0)*10.0)/1000.0;
                  mayaegg_cat.debug() << "after rounding uvs[0]: " << uvs[0] << endl;
                }
                if (uvs[1] > 1.0 || uvs[1] < -1.0) {
                  uvs[1] = (long)(uvs[1]*1000);
                  mayaegg_cat.debug() << "before rounding uvs[1]: " << uvs[1] << endl;
                  uvs[1] = (double)(round((double)uvs[1]/10.0)*10.0)/1000.0;
                  mayaegg_cat.debug() << "after rounding uvs[1]: " << uvs[1] << endl;
                }
              }
              vert.set_uv(panda_uvset_name, LTexCoordd(uvs[0], uvs[1]));
            }
          }
        }
      }

      if (!ignore_vertex_color) {
        if (mayaegg_cat.is_spam()) {
          mayaegg_cat.spam() << "poly_color = " << poly_color << endl;
        }
        set_vertex_color(vert,pi,i,shader,poly_color);
      }

      vert.set_external_index(pi.vertexIndex(i, &status));
      vert.set_external_index2(pi.normalIndex(i, &status));

      egg_poly->add_vertex(vpool->create_unique_vertex(vert));
    }

    // Also get the face normal for the polygon.
    LNormald face_normal;
    bool got_face_normal = false;

    MVector n;
    status = pi.getNormal(n, MSpace::kWorld);
    if (!status) {
      status.perror("MItMeshPolygon::getNormal face");
    } else {
      face_normal.set(n[0], n[1], n[2]);
      face_normal = face_normal * vertex_frame_inv;
      got_face_normal = true;
      egg_poly->set_normal(face_normal);
    }

    // Now, check that the vertex ordering is consistent with the direction of
    // the normals.  If not, reverse the vertex ordering (since we have seen
    // cases where Maya sets this in contradiction to its normals).
    LNormald order_normal;
    if (got_face_normal && egg_poly->calculate_normal(order_normal)) {
      if (order_normal.dot(face_normal) < 0.0) {
        egg_poly->reverse_vertex_ordering();
        if (mayaegg_cat.is_debug()) {
          mayaegg_cat.debug()
            << "reversing polygon\n";
        }
      }
    }

    pi.next();
  }
  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam() << "done traversing polys" << endl;
  }

  // Now that we've added all the polygons (and created all the vertices), go
  // back through the vertex pool and set up the appropriate joint membership
  // for each of the vertices.
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
    // The number of weights should be an even multiple of verts * joints.
    nassertv(num_weights == num_verts * num_joints);

    EggVertexPool::iterator vi;
    for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
      EggVertex *vert = (*vi);
      int maya_vi = vert->get_external_index();
      nassertv(maya_vi >= 0 && maya_vi < num_verts);

      for (int ji = 0; ji < num_joints; ++ji) {
        PN_stdfloat weight = weights[maya_vi * num_joints + ji];
        if (weight != 0.0f) {
          EggGroup *joint = joints[ji];
          if (joint != nullptr) {
            joint->ref_vertex(vert, weight);
          }
        }
      }
    }
  }


  // We also need to compute the vertex morphs for the polyset, based on
  // whatever blend shapes may be present.  This is similar to the code in
  // make_nurbs_surface(), except that since we don't have a one-to-one
  // relationship of egg vertices to Maya vertices, we have to get the morphs
  // down here, after we have added all of the egg vertices.

  if (_animation_convert == AC_model) {
    int num_orig_mesh_verts = mesh.numVertices();

    int num_sliders = node_desc->get_num_blend_descs();
    for (int i = 0; i < num_sliders; i++) {
      MayaBlendDesc *blend_desc = node_desc->get_blend_desc(i);

      // Temporarily push the slider up to 1.0 so we can see what the surface
      // looks like at that value.
      blend_desc->set_slider(1.0);

      // We have to get the mesh object from the dag again after fiddling with
      // the slider.
      MFnMesh blend_mesh(dag_path, &status);
      if (!status) {
        mayaegg_cat.warning()
          << name << " no longer has a mesh after applying "
          << blend_desc->get_name() << "\n";

      } else {
        if (blend_mesh.numVertices() != num_orig_mesh_verts) {
          mayaegg_cat.warning()
            << "Ignoring " << blend_desc->get_name() << " for "
            << name << "; blend shape has " << blend_mesh.numVertices()
            << " vertices while original shape has "
            << num_orig_mesh_verts << ".\n";

        } else {
          MPointArray verts;
          status = blend_mesh.getPoints(verts, MSpace::kWorld);
          if (!status) {
            status.perror("MFnMesh::getPoints");
          } else {
            int num_verts = (int)verts.length();
            EggVertexPool::iterator vi;
            for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
              EggVertex *vert = (*vi);
              int maya_vi = vert->get_external_index();
              nassertv(maya_vi >= 0 && maya_vi < num_verts);

              const MPoint &m = verts[maya_vi];
              LPoint3d m3d(m[0] / m[3], m[1] / m[3], m[2] / m[3]);
              m3d = m3d * vertex_frame_inv;

              LVector3d delta = m3d - vert->get_pos3();
              if (!delta.almost_equal(LVector3d::zero())) {
                EggMorphVertex dxyz(blend_desc->get_name(), delta);
                vert->_dxyzs.insert(dxyz);
              }
            }
          }

          MFloatVectorArray norms;
          status = blend_mesh.getNormals(norms, MSpace::kWorld);
          if (!status) {
            status.perror("MFnMesh::getNormals");
          } else {
            int num_norms = (int)norms.length();
            EggVertexPool::iterator vi;
            for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
              EggVertex *vert = (*vi);
              int maya_vi = vert->get_external_index2();
              nassertv(maya_vi >= 0 && maya_vi < num_norms);

              const MFloatVector &m = norms[maya_vi];
              LVector3d m3d(m[0], m[1], m[2]);
              m3d = m3d * vertex_frame_inv;

              LNormald delta = m3d - vert->get_normal();
              if (!delta.almost_equal(LVector3d::zero())) {
                EggMorphNormal dnormal(blend_desc->get_name(), delta);
                vert->_dnormals.insert(dnormal);
              }
            }
          }
        }
      }

      blend_desc->set_slider(0.0);
    }
  }
}

/**
 * Locators are used in Maya to indicate a particular position in space to the
 * user or the modeler.  We represent that in egg with an ordinary Group node,
 * which we transform by the locator's position, so that the indicated point
 * becomes the origin at this node and below.
 */
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

  // We need to convert the position to world coordinates.  For some reason,
  // Maya can only tell it to us in local coordinates.
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

  egg_group->add_translate3d(p3d);
}

/**
 * Locators are used in Maya to indicate a particular position in space to the
 * user or the modeler.  We represent that in egg with an ordinary Group node,
 * which we transform by the locator's position, so that the indicated point
 * becomes the origin at this node and below.
 */
void MayaToEggConverter::
make_camera_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
             EggGroup *egg_group) {
  MStatus status;

  unsigned int num_children = dag_node.childCount();
  MObject locator;
  bool found_camera = false;
  for (unsigned int ci = 0; ci < num_children && !found_camera; ci++) {
    locator = dag_node.child(ci);
    found_camera = (locator.apiType() == MFn::kCamera);
  }

  if (!found_camera) {
    mayaegg_cat.error()
      << "Couldn't find camera"
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }
  MFnCamera camera (dag_path, &status);
  if ( !status ) {
    status.perror("MFnCamera constructor");
    return;
  }
  MPoint eyePoint = camera.eyePoint(MSpace::kWorld);
  LPoint3d p3d (eyePoint.x, eyePoint.y, eyePoint.z);

  // Now convert the locator point into the group's space.
  p3d = p3d * egg_group->get_node_frame_inv();

  egg_group->add_translate3d(p3d);
}


/**
 * Locators are used in Maya to indicate a particular position in space to the
 * user or the modeler.  We represent that in egg with an ordinary Group node,
 * which we transform by the locator's position, so that the indicated point
 * becomes the origin at this node and below.
 */
void MayaToEggConverter::
make_light_locator(const MDagPath &dag_path, const MFnDagNode &dag_node,
             EggGroup *egg_group) {
  MStatus status;

  unsigned int num_children = dag_node.childCount();
  MObject locator;
  bool found_alight = false;
  bool found_dlight = false;
  bool found_plight = false;
  for (unsigned int ci = 0; ci < num_children && !found_alight && !found_dlight && !found_plight; ci++) {
    locator = dag_node.child(ci);
    found_alight = (locator.apiType() == MFn::kAmbientLight);
    found_dlight = (locator.apiType() == MFn::kDirectionalLight);
    found_plight = (locator.apiType() == MFn::kPointLight);
  }

  if (!found_alight && !found_dlight && !found_plight) {
    mayaegg_cat.error()
      << "Couldn't find light within locator node "
      << dag_path.fullPathName().asChar() << "\n";
    return;
  }

  LPoint3d p3d;

  // We need to convert the position to world coordinates.  For some reason,
  // Maya can only tell it to us in local coordinates.
  MMatrix mat = dag_path.inclusiveMatrix(&status);
  if (!status) {
    status.perror("Can't get coordinate space for light");
    return;
  }
  LMatrix4d n2w(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
                mat[1][0], mat[1][1], mat[1][2], mat[1][3],
                mat[2][0], mat[2][1], mat[2][2], mat[2][3],
                mat[3][0], mat[3][1], mat[3][2], mat[3][3]);
  p3d = p3d * n2w;

  // Now convert the locator point into the group's space.
  p3d = p3d * egg_group->get_node_frame_inv();

  egg_group->add_translate3d(p3d);
}


/**
 *
 */
bool MayaToEggConverter::
get_vertex_weights(const MDagPath &dag_path, const MFnMesh &mesh,
                   pvector<EggGroup *> &joints, MFloatArray &weights) {
  MStatus status;

  // Since we are working with a mesh the input attribute that creates the
  // mesh is named "inMesh"
  MObject attr = mesh.attribute("inMesh");

  // Create the plug to the "inMesh" attribute then use the DG iterator to
  // walk through the DG, at the node level.
  MPlug history(mesh.object(), attr);
  MItDependencyGraph it(history, MFn::kDependencyNode,
                        MItDependencyGraph::kUpstream,
                        MItDependencyGraph::kDepthFirst,
                        MItDependencyGraph::kNodeLevel);

  while (!it.isDone()) {
    // We will walk along the node level of the DG until we spot a skinCluster
    // node.
    MObject c_node = it.thisNode();
    if (c_node.hasFn(MFn::kSkinClusterFilter)) {
      // We've found the cluster handle.  Try to get the weight data.
      MFnSkinCluster cluster(c_node, &status);
      if (!status) {
        status.perror("MFnSkinCluster constructor");
        return false;
      }

      // Get the set of objects that influence the vertices of this mesh.
      // Hopefully these will all be joints.
      MDagPathArray influence_objects;
      cluster.influenceObjects(influence_objects, &status);
      if (!status) {
        status.perror("MFnSkinCluster::influenceObjects");

      } else {
        // Fill up the vector with the corresponding table of egg groups for
        // each joint.
        joints.clear();
        for (unsigned oi = 0; oi < influence_objects.length(); oi++) {
          MDagPath joint_dag_path = influence_objects[oi];
          MayaNodeDesc *joint_node_desc = _tree.build_node(joint_dag_path);
          EggGroup *joint = _tree.get_egg_group(joint_node_desc);
          joints.push_back(joint);
        }

        // Now use a component object to retrieve all of the weight data in
        // one API call.
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
            // We've got the weights and the set of objects.  That's all we
            // need.
            return true;
          }
        }
      }
    } else if (c_node.hasFn(MFn::kWeightGeometryFilt)) {
      // We've found the joint cluster handle.  (rigid Binding)
      MFnWeightGeometryFilter cluster(c_node, &status);
      if (!status) {
        status.perror("MFnWeightGeometryFilter constructor");
        return false;
      }

      MPlug matrix_plug = cluster.findPlug("matrix");
      if (!matrix_plug.isNull()) {
        MPlugArray matrix_pa;
        matrix_plug.connectedTo(matrix_pa, true, false, &status);
        if (!status) {
          status.perror("Can't find connected Joint");
        } else {
          MObject jointObj = matrix_pa[0].node();
          MFnIkJoint jointFn(jointObj, &status);
          if (!status) {
            status.perror("Can't find connected JointDag");
          } else {
            joints.clear();
            MDagPath joint_dag_path = MDagPath();
            status = jointFn.getPath(joint_dag_path);
            if (!status) {
              status.perror("MFnIkJoint::dagPath");
            } else {
              MayaNodeDesc *joint_node_desc = _tree.build_node(joint_dag_path);
              EggGroup *joint = _tree.get_egg_group(joint_node_desc);
              joints.push_back(joint);

              // Now use a component object to retrieve all of the weight data
              // in one API call.
              MFnSingleIndexedComponent sic;
              MObject sic_object = sic.create(MFn::kMeshVertComponent);
              sic.setCompleteData(mesh.numVertices());

              status = cluster.getWeights(dag_path, sic_object,
                                          weights);
              if (!status) {
                status.perror("MFnWeightGeometryFilter::getWeights");
              } else {
                // We've got the weights and the set of objects.  That's all
                // we need.
                return true;
              }
            }
          }
        }
      }
    }

    it.next();
  }

  // The mesh was not soft-skinned.
  return false;
}

/**
 * As above, for a NURBS surface instead of a polygon mesh.
 */
bool MayaToEggConverter::
get_vertex_weights(const MDagPath &dag_path, const MFnNurbsSurface &surface,
                   pvector<EggGroup *> &joints, MFloatArray &weights) {
  MStatus status;

  // Since we are working with a NURBS surface the input attribute that
  // creates the surface is named "create"
  MObject attr = surface.attribute("create");

  // Create the plug to the "create" attribute then use the DG iterator to
  // walk through the DG, at the node level.
  MPlug history(surface.object(), attr);
  MItDependencyGraph it(history, MFn::kDependencyNode,
                        MItDependencyGraph::kUpstream,
                        MItDependencyGraph::kDepthFirst,
                        MItDependencyGraph::kNodeLevel);

  while (!it.isDone()) {
    // We will walk along the node level of the DG until we spot a skinCluster
    // node.
    MObject c_node = it.thisNode();
    if (c_node.hasFn(MFn::kSkinClusterFilter)) {
      // We've found the cluster handle.  Try to get the weight data.
      MFnSkinCluster cluster(c_node, &status);
      if (!status) {
        status.perror("MFnSkinCluster constructor");
        return false;
      }

      // Get the set of objects that influence the vertices of this surface.
      // Hopefully these will all be joints.
      MDagPathArray influence_objects;
      cluster.influenceObjects(influence_objects, &status);
      if (!status) {
        status.perror("MFnSkinCluster::influenceObjects");

      } else {
        // Fill up the vector with the corresponding table of egg groups for
        // each joint.
        joints.clear();
        for (unsigned oi = 0; oi < influence_objects.length(); oi++) {
          MDagPath joint_dag_path = influence_objects[oi];
          MayaNodeDesc *joint_node_desc = _tree.build_node(joint_dag_path);
          EggGroup *joint = _tree.get_egg_group(joint_node_desc);
          joints.push_back(joint);
        }

        // Now use a component object to retrieve all of the weight data in
        // one API call.
        MFnDoubleIndexedComponent dic;
        MObject dic_object = dic.create(MFn::kSurfaceCVComponent);
        dic.setCompleteData(surface.numCVsInU(), surface.numCVsInV());
        unsigned influence_count;

        status = cluster.getWeights(dag_path, dic_object,
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
            // We've got the weights and the set of objects.  That's all we
            // need.
            return true;
          }
        }
      }
    }

    it.next();
  }

  // The surface was not soft-skinned.
  return false;
}

/**
 * Applies the known shader attributes to the indicated egg primitive.  Note:
 * For multi-textures, Maya lists the top most texture in slot 0. But Panda
 * puts the base texture at slot 0. Hence I parse the list of textures from
 * last to first.
 */
void MayaToEggConverter::
set_shader_attributes(EggPrimitive &primitive, const MayaShader &shader,
                      bool mesh) {
  if (shader._legacy_mode) {
    set_shader_legacy(primitive, shader, mesh);
  } else {
    set_shader_modern(primitive, shader, mesh);
  }
}

/**
 * The modern implementation of set_shader_attributes.
 *
 * In the modern codepath, the MayaShader is a direct, literal representation
 * of a list of EggTextures.  All this exporter has to do is translate the
 * list without interpretation.  All the complex interpretation is handled
 * elsewhere, in the MayaShader module.
 */
void MayaToEggConverter::
set_shader_modern(EggPrimitive &primitive, const MayaShader &shader,
                      bool mesh) {

  for (size_t idx=0; idx < shader._all_maps.size(); idx++) {
    MayaShaderColorDef *def = shader._all_maps[idx];
    if ((def->_is_alpha)&&(def->_opposite != 0)) {
      // This texture represents an alpha-filename.  It doesn't get its own
      // <Texture>
      continue;
    }

    EggTexture tex(shader.get_name(), "");
    tex.set_format(def->_is_alpha ? EggTexture::F_alpha : EggTexture::F_rgb);
    apply_texture_filename(tex, *def);
    if (def->_opposite) {
      apply_texture_alpha_filename(tex, *def);
    }
    apply_texture_uvprops(tex, *def);
    apply_texture_blendtype(tex, *def);
    tex.set_uv_name(def->get_panda_uvset_name());

    EggTexture *new_tex =
      _textures.create_unique_texture(tex, ~0);
    primitive.add_texture(new_tex);
  }
}

/**
 * The legacy implementation of set_shader_attributes.  The old behavior of
 * the exporter is just plain weird.  It seems to be a result of an
 * inexperienced coder who made some core mistakes, and then patched them up
 * with kludges.  It seems to produce plausible results in certain specific
 * cases, but overall, it doesn't make any sense.  Unfortunately, this weird
 * behavior cannot be discarded - vast numbers of 3D models have been created
 * that rely on this behavior.  The solution is to compartmentalize the
 * weirdness.  The legacy codepath, when activated, implements the old weird
 * behavior.  A brand-new codepath that shares almost nothing with the legacy
 * codepath implements a much more straightforward behavior.
 */
void MayaToEggConverter::
set_shader_legacy(EggPrimitive &primitive, const MayaShader &shader,
                      bool mesh) {

  // determine if the base texture or any of the top texture need to be rgb
  // only
  MayaShaderColorDef *color_def = nullptr;
  bool is_rgb = false;
  bool is_decal = false;
  bool is_interpolate = false;
  int i;
  // last shader is the base so lets skip it
  for (i=0; i<(int)shader._color.size()-1; ++i) {
    color_def = shader.get_color_def(i);
    if (color_def->_has_texture) {
      if ((EggTexture::EnvType)color_def->_interpolate) {
        is_interpolate = true;
      }
      else if ((EggTexture::EnvType)color_def->_blend_type == EggTexture::ET_modulate) {
        // Maya's multiply is slightly different than panda's.  Unless,
        // _keep_alpha is set, we are dropping the alpha.
        if (!color_def->_keep_alpha)
          is_rgb = true; // modulate forces the alpha to be ignored
      }
      else if ((EggTexture::EnvType)color_def->_blend_type == EggTexture::ET_decal) {
        is_decal = true;
      }
    }
  }

  // we don't want an extra light stage for interpolate mode, it takes care of
  // automatically
  if (is_interpolate)
    is_decal = false;

  // new decal mode needs an extra dummy layers of textureStage
  EggTexture *dummy_tex = nullptr;
  string dummy_uvset_name;

  // In Maya, a polygon is either textured or colored.  The texture, if
  // present, replaces the color.  Also now there could be multiple textures
  const MayaShaderColorDef &trans_def = shader._transparency;
  for (i=shader._color.size()-1; i>=0; --i) {
    color_def = shader.get_color_def(i);
    if (mayaegg_cat.is_spam()) {
      mayaegg_cat.spam() << "slot " << i << ":got color_def: " << color_def << endl;
    }
    if (color_def->_has_texture || trans_def._has_texture) {
      EggTexture tex(shader.get_name(), "");
      if (mayaegg_cat.is_spam()) {
        mayaegg_cat.spam() << "got shader name:" << shader.get_name() << endl;
        mayaegg_cat.spam() << "ssa:texture name[" << i << "]: " << color_def->_texture_name << endl;
      }

      string uvset_name = _shaders.find_uv_link(color_def->_texture_name);
      if (mayaegg_cat.is_spam()) {
        mayaegg_cat.spam() << "ssa:corresponding uvset name is " << uvset_name << endl;
      }

      if (color_def->_has_texture) {
        // If we have a texture on color, apply it as the filename.  if
        // (mayaegg_cat.is_debug()) { mayaegg_cat.debug() << "ssa:got texture
        // name" << color_def->_texture_filename << endl; }
        Filename filename = Filename::from_os_specific(color_def->_texture_filename);
        Filename fullpath, outpath;
        _path_replace->full_convert_path(filename, get_model_path(), fullpath, outpath);
        tex.set_filename(outpath);
        tex.set_fullpath(fullpath);
        apply_texture_uvprops(tex, *color_def);

        // If we also have a texture on transparency, apply it as the alpha
        // filename.
        if (trans_def._has_texture) {
          if (color_def->_wrap_u != trans_def._wrap_u ||
              color_def->_wrap_u != trans_def._wrap_u) {
            mayaegg_cat.warning()
              << "Shader " << shader.get_name()
              << " has contradictory wrap modes on color and texture.\n";
          }

          if (!compare_texture_uvprops(tex, trans_def)) {
            // Only report each broken shader once.
            static pset<string> bad_shaders;
            if (bad_shaders.insert(shader.get_name()).second) {
              mayaegg_cat.error()
                << "Color and transparency texture properties differ on shader "
                << shader.get_name() << "\n";
            }
          }
          // tex.set_format(EggTexture::F_rgba);

          // We should try to be smarter about whether the transparency value
          // is connected to the texture's alpha channel or to its grayscale
          // channel.  However, I'm not sure how to detect this at the moment;
          // rather than spending days trying to figure out, for now I'll just
          // assume that if the same texture image is used for both color and
          // transparency, then the artist meant to use the alpha channel for
          // transparency.
          if (trans_def._texture_filename == color_def->_texture_filename) {
            // That means that we don't need to do anything special: use all
            // the channels of the texture.

          } else {
            // Otherwise, pull the alpha channel from the other image file.
            // Ideally, we should figure out which channel from the other
            // image supplies alpha (and specify this via
            // set_alpha_file_channel()), but for now we assume it comes from
            // the grayscale data.
            filename = Filename::from_os_specific(trans_def._texture_filename);
            _path_replace->full_convert_path(filename, get_model_path(),
                                             fullpath, outpath);
            tex.set_alpha_filename(outpath);
            tex.set_alpha_fullpath(fullpath);
          }
        } else {
          // If there is no transparency texture specified, we don't have any
          // transparency, so tell the egg format to ignore any alpha channel
          // that might be on the color texture.
          // tex.set_format(EggTexture::F_rgb);
        }

        if (shader._color.size() > 1) {
          // if multi-textured, first texture in maya is on top, so last
          // shader on the list is the base one, which should always pick up
          // the alpha from the texture file.  But the top textures may have
          // to strip the alpha
          if ((size_t)i != shader._color.size() - 1) {
            if (!i && is_interpolate) {
              // this is the grass path mode where alpha on this texture
              // determines whether to show layer1 or layer2. Since by now
              // other layers are set lets change those to get this effect
              tex.set_combine_mode(EggTexture::CC_rgb, EggTexture::CM_interpolate);
              tex.set_combine_source(EggTexture::CC_rgb, 0, EggTexture::CS_previous);
              tex.set_combine_operand(EggTexture::CC_rgb, 0, EggTexture::CO_src_color);
              tex.set_combine_source(EggTexture::CC_rgb, 1, EggTexture::CS_last_saved_result);
              tex.set_combine_operand(EggTexture::CC_rgb, 1, EggTexture::CO_src_color);
              tex.set_combine_source(EggTexture::CC_rgb, 2, EggTexture::CS_texture);
              tex.set_combine_operand(EggTexture::CC_rgb, 2, EggTexture::CO_src_alpha);

              tex.set_combine_mode(EggTexture::CC_alpha, EggTexture::CM_interpolate);
              tex.set_combine_source(EggTexture::CC_alpha, 0, EggTexture::CS_previous);
              tex.set_combine_operand(EggTexture::CC_alpha, 0, EggTexture::CO_src_alpha);
              tex.set_combine_source(EggTexture::CC_alpha, 1, EggTexture::CS_last_saved_result);
              tex.set_combine_operand(EggTexture::CC_alpha, 1, EggTexture::CO_src_alpha);
              tex.set_combine_source(EggTexture::CC_alpha, 2, EggTexture::CS_texture);
              tex.set_combine_operand(EggTexture::CC_alpha, 2, EggTexture::CO_src_alpha);
            }
            else {
              if (is_interpolate) {
                tex.set_combine_mode(EggTexture::CC_rgb, EggTexture::CM_modulate);
                tex.set_combine_source(EggTexture::CC_rgb, 0, EggTexture::CS_primary_color);
                tex.set_combine_operand(EggTexture::CC_rgb, 0, EggTexture::CO_src_color);
                tex.set_combine_source(EggTexture::CC_rgb, 1, EggTexture::CS_texture);
                tex.set_combine_operand(EggTexture::CC_rgb, 1, EggTexture::CO_src_color);
              }
              else {
                tex.set_env_type((EggTexture::EnvType)color_def->_blend_type);
                if (tex.get_env_type() == EggTexture::ET_modulate) {
                  if (color_def->_has_alpha_channel) {
                    // lets caution the artist that they should not be using a
                    // alpha channel on this texture.
                    if (mayaegg_cat.is_spam()) {
                      maya_cat.spam()
                        << color_def->_texture_name
                        << " should not have alpha channel in multiply mode: ignoring\n";
                    }
                  }
                  if (is_rgb) {
                    // tex.set_alpha_mode(EggRenderMode::AM_off);   force
                    // alpha off
                    tex.set_format(EggTexture::F_rgb);  // Change the format to be rgb only
                  }
                }
              }
            }
          }
          else {
            if (is_interpolate) {
              // base shader need to save result
              tex.set_saved_result(true);
            }
            else if (is_decal) {
              // decal in classic time, always overwrote the base color.  That
              // causes problem when the polygon wants to be lit or wants to
              // retain vertexpolygon color In the new decal mode, we achieve
              // this with a third dummy layer copy this layer to a new dummy
              // layer
              EggTexture texDummy(shader.get_name()+".dummy", "");
              if (mayaegg_cat.is_debug()) {
                mayaegg_cat.debug() << "creating dummy shader: " << texDummy.get_name() << endl;
              }
              texDummy.set_filename(outpath);
              texDummy.set_fullpath(fullpath);
              apply_texture_uvprops(texDummy, *color_def);
              texDummy.set_combine_mode(EggTexture::CC_rgb, EggTexture::CM_modulate);
              texDummy.set_combine_source(EggTexture::CC_rgb, 0, EggTexture::CS_primary_color);
              texDummy.set_combine_operand(EggTexture::CC_rgb, 0, EggTexture::CO_src_color);
              texDummy.set_combine_source(EggTexture::CC_rgb, 1, EggTexture::CS_previous);
              texDummy.set_combine_operand(EggTexture::CC_rgb, 1, EggTexture::CO_src_color);
              dummy_tex = _textures.create_unique_texture(texDummy, ~0);

              // make this layer ET_replace
              tex.set_env_type(EggTexture::ET_replace);
            }
          }
        }
      } else {  // trans_def._has_texture
        // We have a texture on transparency only.  Apply it as the primary
        // filename, and set the format accordingly.
        Filename filename = Filename::from_os_specific(trans_def._texture_filename);
        Filename fullpath,outpath;
        _path_replace->full_convert_path(filename, get_model_path(),
                                         fullpath, outpath);
        tex.set_filename(outpath);
        tex.set_fullpath(fullpath);
        tex.set_format(EggTexture::F_alpha);
        apply_texture_uvprops(tex, trans_def);
      }

      if (mayaegg_cat.is_debug()) {
        mayaegg_cat.debug() << "ssa:tref_name:" << tex.get_name() << endl;
      }
      if (is_rgb && i == (int)shader._color.size()-1) {
        // make base layer rgb only
        tex.set_format(EggTexture::F_rgb);  // Change the format to be rgb only
      }
      EggTexture *new_tex =
        _textures.create_unique_texture(tex, ~0);

      if (mesh) {
        if (uvset_name.find("not found") == string::npos) {
          primitive.add_texture(new_tex);
          color_def->_uvset_name.assign(uvset_name.c_str());
          if (uvset_name != "map1") {
            new_tex->set_uv_name(uvset_name);
          }
          if (i == (int)shader._color.size()-1 && is_decal) {
            dummy_uvset_name.assign(color_def->_uvset_name);
          }
        }
      } else {
        primitive.add_texture(new_tex);
        if (color_def->_uvset_name != "map1") {
          new_tex->set_uv_name(color_def->_uvset_name);
        }
      }
    }
  }
  if (dummy_tex != nullptr) {
    primitive.add_texture(dummy_tex);
    dummy_tex->set_uv_name(dummy_uvset_name);
  }
  // Also apply an overall color to the primitive.
  LColor rgba = shader.get_rgba();
  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam() << "ssa:rgba = " << rgba << endl;
  }

  // The existence of a texture on either color channel completely replaces
  // the corresponding flat color.
  if (color_def && color_def->_has_texture) {
    rgba[0] = 1.0f;
    rgba[1] = 1.0f;
    rgba[2] = 1.0f;
  }
  if (trans_def._has_texture) {
    rgba[3] = 1.0f;
  }

  // But the color gain always gets applied.
  if (color_def) {
    rgba[0] *= color_def->_color_gain[0];
    rgba[1] *= color_def->_color_gain[1];
    rgba[2] *= color_def->_color_gain[2];
    rgba[3] *= color_def->_color_gain[3];
    if (mayaegg_cat.is_spam()) {
      mayaegg_cat.spam() << "ssa:rgba = " << rgba << endl;
    }
  }

  primitive.set_color(rgba);

  if (mayaegg_cat.is_spam()) {
    mayaegg_cat.spam() << "  set_shader_attributes : end\n";
  }
}

/**
 * Applies all the appropriate texture properties to the EggTexture object,
 * including wrap modes and texture matrix.
 */
void MayaToEggConverter::
apply_texture_uvprops(EggTexture &tex, const MayaShaderColorDef &color_def) {
  // Let's mipmap all textures by default.
  tex.set_minfilter(EggTexture::FT_linear_mipmap_linear);
  tex.set_magfilter(EggTexture::FT_linear);

  EggTexture::WrapMode wrap_u = color_def._wrap_u ? EggTexture::WM_repeat : EggTexture::WM_clamp;
  EggTexture::WrapMode wrap_v = color_def._wrap_v ? EggTexture::WM_repeat : EggTexture::WM_clamp;

  tex.set_wrap_u(wrap_u);
  tex.set_wrap_v(wrap_v);

  LMatrix3d mat = color_def.compute_texture_matrix();
  if (!mat.almost_equal(LMatrix3d::ident_mat())) {
    tex.set_transform2d(mat);
  }
}

/**
 * Applies the blendtype to the EggTexture.
 */
void MayaToEggConverter::
apply_texture_blendtype(EggTexture &tex, const MayaShaderColorDef &color_def) {
  switch (color_def._blend_type) {
  case MayaShaderColorDef::BT_unspecified:
    tex.set_env_type(EggTexture::ET_unspecified);
    return;
  case MayaShaderColorDef::BT_modulate:
    tex.set_env_type(EggTexture::ET_modulate);
    return;
  case MayaShaderColorDef::BT_decal:
    tex.set_env_type(EggTexture::ET_decal);
    return;
  case MayaShaderColorDef::BT_blend:
    tex.set_env_type(EggTexture::ET_blend);
    return;
  case MayaShaderColorDef::BT_replace:
    tex.set_env_type(EggTexture::ET_replace);
    return;
  case MayaShaderColorDef::BT_add:
    tex.set_env_type(EggTexture::ET_add);
    return;
  case MayaShaderColorDef::BT_blend_color_scale:
    tex.set_env_type(EggTexture::ET_blend_color_scale);
    return;
  case MayaShaderColorDef::BT_modulate_glow:
    tex.set_env_type(EggTexture::ET_modulate_glow);
    return;
  case MayaShaderColorDef::BT_modulate_gloss:
    tex.set_env_type(EggTexture::ET_modulate_gloss);
    return;
  case MayaShaderColorDef::BT_normal:
    tex.set_env_type(EggTexture::ET_normal);
    return;
  case MayaShaderColorDef::BT_normal_height:
    tex.set_env_type(EggTexture::ET_normal_height);
    return;
  case MayaShaderColorDef::BT_glow:
    tex.set_env_type(EggTexture::ET_glow);
    return;
  case MayaShaderColorDef::BT_gloss:
    tex.set_env_type(EggTexture::ET_gloss);
    return;
  case MayaShaderColorDef::BT_height:
    tex.set_env_type(EggTexture::ET_height);
    return;
  case MayaShaderColorDef::BT_selector:
    tex.set_env_type(EggTexture::ET_selector);
    return;
  }
}

/**
 * Applies the filename to the EggTexture.
 */
void MayaToEggConverter::
apply_texture_filename(EggTexture &tex, const MayaShaderColorDef &def) {
  Filename filename = Filename::from_os_specific(def._texture_filename);
  Filename fullpath, outpath;
  _path_replace->full_convert_path(filename, get_model_path(), fullpath, outpath);
  tex.set_filename(outpath);
  tex.set_fullpath(fullpath);
}

/**
 * Applies the alpha filename to the EggTexture.
 */
void MayaToEggConverter::
apply_texture_alpha_filename(EggTexture &tex, const MayaShaderColorDef &def) {
  if (def._opposite) {
    tex.set_format(EggTexture::F_rgba);
    if (def._opposite->_texture_filename != def._texture_filename) {
      Filename filename = Filename::from_os_specific(def._opposite->_texture_filename);
      Filename fullpath, outpath;
      _path_replace->full_convert_path(filename, get_model_path(), fullpath, outpath);
      tex.set_alpha_filename(outpath);
      tex.set_alpha_fullpath(fullpath);
    }
  }
}

/**
 * Compares the texture properties already on the texture (presumably set by a
 * previous call to apply_texture_uvprops()) and returns false if they differ
 * from that specified by the indicated color_def object, or true if they
 * match.
 */
bool MayaToEggConverter::
compare_texture_uvprops(EggTexture &tex,
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

  LMatrix3d m = color_def.compute_texture_matrix();
  LMatrix4d mat4(m(0, 0), m(0, 1), 0.0, m(0, 2),
                 m(1, 0), m(1, 1), 0.0, m(1, 2),
                 0.0, 0.0, 1.0, 0.0,
                 m(2, 0), m(2, 1), 0.0, m(2, 2));
  if (!mat4.almost_equal(tex.get_transform3d())) {
    okflag = false;
  }

  return okflag;
}

/**
 * Recursively walks the egg hierarchy, reparenting "decal" type nodes below
 * their corresponding "decalbase" type nodes, and setting the flags.
 *
 * Returns true on success, false if some nodes were incorrect.
 */
bool MayaToEggConverter::
reparent_decals(EggGroupNode *egg_parent) {
  bool okflag = true;

  // First, walk through all children of this node, looking for the one decal
  // base, if any.
  EggGroup *decal_base = nullptr;
  pvector<EggGroup *> decal_children;

  EggGroupNode::iterator ci;
  for (ci = egg_parent->begin(); ci != egg_parent->end(); ++ci) {
    EggNode *child =  (*ci);
    if (child->is_of_type(EggGroup::get_class_type())) {
      EggGroup *child_group = DCAST(EggGroup, child);
      if (child_group->has_object_type("decalbase")) {
        if (decal_base != nullptr) {
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

  if (decal_base == nullptr) {
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
      // All the decal children get moved to be a child of decal base.  This
      // usually will not affect the vertex positions, but it could if the
      // decal base has a transform and the decal child is an instance node.
      // So don't do that.
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

/**
 * Returns the TransformType value corresponding to the indicated string, or
 * TT_invalid.
 */
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
/**
 * Checks to see if we're using legacy or modern shaders and based on the
 * result, it passes the vertex color calculations off to either the legacy or
 * modern vertex color functions.
 */
void MayaToEggConverter::
set_vertex_color(EggVertex &vert, MItMeshPolygon &pi, int vert_index, const MayaShader *shader, const LColor &color) {
    if (shader == nullptr || shader->_legacy_mode) {
      set_vertex_color_legacy(vert, pi, vert_index, shader, color);
    } else {
      set_vertex_color_modern(vert, pi, vert_index, shader, color);
    }
}
/**
 * Calls set_color on an EggVertex, determining the correct color values,
 * based on the shader, vert_color Maya's vertex & flat color(s).  This is the
 * original implementation that works only on Lambert shaders/materials in
 * Maya.
 */
void MayaToEggConverter::
set_vertex_color_legacy(EggVertex &vert, MItMeshPolygon &pi, int vert_index, const MayaShader *shader, const LColor &color){
  if (pi.hasColor()) {
    MColor c;
    MStatus status = pi.getColor(c, vert_index);
    if (!status) {
      status.perror("MItMeshPolygon::getColor");
    } else {
      // I saw instances where the color components exceeded 1.0 so lets clamp
      // the values to 0 to 1
      c /= 1.0;
      // The vertex color is a color scale that modifies the polygon color,
      // not an override that replaces it.
      vert.set_color(LColor(c.r * color[0], c.g * color[1], c.b * color[2], c.a * color[3]));

      if (mayaegg_cat.is_spam()) {
        mayaegg_cat.spam() << "maya_color = " << c.r << " " << c.g << " " << c.b << " " << c.a << endl;
        mayaegg_cat.spam() << "vert_color = " << vert.get_color() << endl;
      }
    }
  } else {
    vert.set_color(color);
  }

}
/**
 * Calls set_color on an EggVertex, determining the correct color values,
 * based on the shader, vert_color Maya's vertex & flat color(s).  This
 * implementation is designed to work specifically with Phong materials or
 * shaders.
 */
void MayaToEggConverter::
set_vertex_color_modern(EggVertex &vert, MItMeshPolygon &pi, int vert_index, const MayaShader *shader, const LColor &color) {
  // If there's an explicit vertex color, output it.
  if (pi.hasColor(vert_index)) {
    MColor c;
    MStatus status = pi.getColor(c, vert_index);
    if (status) {
      vert.set_color(LColor(c.r, c.g, c.b, c.a));
      return;
    }
  }

  // If there's no explicit color, use flat color, or white on a textured
  // model.
  if (shader->_color_maps.empty()) {
    const LColord &c = shader->_flat_color;
    vert.set_color(LColor((PN_stdfloat)c[0], (PN_stdfloat)c[1], (PN_stdfloat)c[2], (PN_stdfloat)c[3]));
  } else {
    // there's no explicit color anywhere, must be textured (or blank)
    vert.set_color(LColor(1.0f, 1.0f, 1.0f, 1.0f));
  }
}
