// Filename: camera.cxx
// Created by:  drose (26Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"
#include "camera.h"
#include "lens.h"
#include "throw_event.h"

TypeHandle Camera::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: Camera::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Camera::
Camera(const string &name, Lens *lens) :
  LensNode(name, lens),
  _active(true),
  _camera_mask(~PandaNode::get_overall_bit()),
  _initial_state(RenderState::make_empty())
{
  set_lod_scale(1.0);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
Camera::
Camera(const Camera &copy) :
  LensNode(copy),
  _active(copy._active),
  _scene(copy._scene),
  _camera_mask(copy._camera_mask),
  _initial_state(copy._initial_state),
  _lod_scale(copy._lod_scale),
  _tag_state_key(copy._tag_state_key),
  _tag_states(copy._tag_states)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Camera::
~Camera() {
  // We don't have to destroy the display region(s) associated with
  // the camera; they're responsible for themselves.  However, they
  // should have removed themselves before we destruct, or something
  // went wrong.
  nassertv(_display_regions.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *Camera::
make_copy() const {
  return new Camera(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool Camera::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::safe_to_transform
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to transform
//               this particular kind of Node by calling the xform()
//               method, false otherwise.  For instance, it's usually
//               a bad idea to attempt to xform a Character.
////////////////////////////////////////////////////////////////////
bool Camera::
safe_to_transform() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_tag_state
//       Access: Published
//  Description: Associates a particular state transition with the
//               indicated tag value.  When a node is encountered
//               during traversal with the tag key specified by
//               set_tag_state_key(), if the value of that tag matches
//               tag_state, then the indicated state is applied to
//               this node--but only when it is rendered by this
//               camera.
//
//               This can be used to apply special effects to nodes
//               when they are rendered by certain cameras.  It is
//               particularly useful for multipass rendering, in which
//               specialty cameras might be needed to render the scene
//               with a particular set of effects.
////////////////////////////////////////////////////////////////////
void Camera::
set_tag_state(const string &tag_state, const RenderState *state) {
  _tag_states[tag_state] = state;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::clear_tag_state
//       Access: Published
//  Description: Removes the association established by a previous
//               call to set_tag_state().
////////////////////////////////////////////////////////////////////
void Camera::
clear_tag_state(const string &tag_state) {
  _tag_states.erase(tag_state);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::has_tag_state
//       Access: Published
//  Description: Returns true if set_tag_state() has previously been
//               called with the indicated tag state, false otherwise.
////////////////////////////////////////////////////////////////////
bool Camera::
has_tag_state(const string &tag_state) const {
  TagStates::const_iterator tsi;
  tsi = _tag_states.find(tag_state);
  return (tsi != _tag_states.end());
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_tag_state
//       Access: Published
//  Description: Returns the state associated with the indicated tag
//               state by a previous call to set_tag_state(), or the
//               empty state if nothing has been associated.
////////////////////////////////////////////////////////////////////
CPT(RenderState) Camera::
get_tag_state(const string &tag_state) const {
  TagStates::const_iterator tsi;
  tsi = _tag_states.find(tag_state);
  if (tsi != _tag_states.end()) {
    return (*tsi).second;
  }
  return RenderState::make_empty();
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::set_aux_scene_data
//       Access: Published
//  Description: Associates the indicated AuxSceneData object with the
//               given NodePath, possibly replacing a previous
//               data defined for the same NodePath, if any.
////////////////////////////////////////////////////////////////////
void Camera::
set_aux_scene_data(const NodePath &node_path, AuxSceneData *data) {
  if (data == (AuxSceneData *)NULL) {
    clear_aux_scene_data(node_path);
  } else {
    _aux_data[node_path] = data;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::clear_aux_scene_data
//       Access: Published
//  Description: Removes the AuxSceneData associated with the
//               indicated NodePath.  Returns true if it is removed
//               successfully, false if it was already gone.
////////////////////////////////////////////////////////////////////
bool Camera::
clear_aux_scene_data(const NodePath &node_path) {
  AuxData::iterator ai;
  ai = _aux_data.find(node_path);
  if (ai != _aux_data.end()) {
    _aux_data.erase(ai);
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::get_aux_scene_data
//       Access: Published
//  Description: Returns the AuxSceneData associated with the
//               indicated NodePath, or NULL if nothing is associated.
////////////////////////////////////////////////////////////////////
AuxSceneData *Camera::
get_aux_scene_data(const NodePath &node_path) const {
  AuxData::const_iterator ai;
  ai = _aux_data.find(node_path);
  if (ai != _aux_data.end()) {
    return (*ai).second;
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::list_aux_scene_data
//       Access: Published
//  Description: Outputs all of the NodePaths and AuxSceneDatas in
//               use.
////////////////////////////////////////////////////////////////////
void Camera::
list_aux_scene_data(ostream &out) const {
  out << _aux_data.size() << " data objects held:\n";
  AuxData::const_iterator ai;
  for (ai = _aux_data.begin(); ai != _aux_data.end(); ++ai) {
    out << (*ai).first << " " << *(*ai).second << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::cleanup_aux_scene_data
//       Access: Published
//  Description: Walks through the list of currently-assigned
//               AuxSceneData objects and releases any that are
//               past their expiration times.  Returns the number of
//               elements released.
////////////////////////////////////////////////////////////////////
int Camera::
cleanup_aux_scene_data(Thread *current_thread) {
  int num_deleted = 0;

  double now = ClockObject::get_global_clock()->get_frame_time(current_thread);

  AuxData::iterator ai;
  ai = _aux_data.begin();
  while (ai != _aux_data.end()) {
    AuxData::iterator anext = ai;
    ++anext;

    if (now > (*ai).second->get_expiration_time()) {
      _aux_data.erase(ai);
      num_deleted++;
    }

    ai = anext;
  }

  return num_deleted;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::add_display_region
//       Access: Private
//  Description: Adds the indicated DisplayRegion to the set of
//               DisplayRegions shared by the camera.  This is only
//               intended to be called from the DisplayRegion.
////////////////////////////////////////////////////////////////////
void Camera::
add_display_region(DisplayRegionBase *display_region) {
  _display_regions.push_back(display_region);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::remove_display_region
//       Access: Private
//  Description: Removes the indicated DisplayRegion from the set of
//               DisplayRegions shared by the camera.  This is only
//               intended to be called from the DisplayRegion.
////////////////////////////////////////////////////////////////////
void Camera::
remove_display_region(DisplayRegionBase *display_region) {
  DisplayRegions::iterator dri =
    find(_display_regions.begin(), _display_regions.end(), display_region);
  if (dri != _display_regions.end()) {
    _display_regions.erase(dri);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Camera.
////////////////////////////////////////////////////////////////////
void Camera::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Camera::
write_datagram(BamWriter *manager, Datagram &dg) {
  LensNode::write_datagram(manager, dg);

  dg.add_bool(_active);
  dg.add_uint32(_camera_mask.get_word());
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Camera is encountered
//               in the Bam file.  It should create the Camera
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *Camera::
make_from_bam(const FactoryParams &params) {
  Camera *node = new Camera("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: Camera::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Camera.
////////////////////////////////////////////////////////////////////
void Camera::
fillin(DatagramIterator &scan, BamReader *manager) {
  LensNode::fillin(scan, manager);

  _active = scan.get_bool();
  _camera_mask.set_word(scan.get_uint32());
}
