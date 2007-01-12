// Filename: character.cxx
// Created by:  drose (06Mar02)
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

#include "character.h"
#include "characterJoint.h"
#include "config_char.h"

#include "geomNode.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pStatTimer.h"
#include "animControl.h"
#include "clockObject.h"
#include "pStatTimer.h"

TypeHandle Character::_type_handle;

PStatCollector Character::_animation_pcollector("*:Animation");

////////////////////////////////////////////////////////////////////
//     Function: Character::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy a Character.
////////////////////////////////////////////////////////////////////
Character::
Character(const Character &copy) :
  PartBundleNode(copy),
  _joints_pcollector(copy._joints_pcollector),
  _skinning_pcollector(copy._skinning_pcollector)
{
  set_cull_callback();

  // Copy the bundle(s).
  int num_bundles = copy.get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    PartBundle *orig_bundle = copy.get_bundle(i);
    PartBundle *new_bundle = 
      new CharacterJointBundle(orig_bundle->get_name());
    add_bundle(new_bundle);

    // Make a copy of the joint/slider hierarchy.
    copy_joints(new_bundle, orig_bundle);
  }

  _last_auto_update = -1.0;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Character::
Character(const string &name) :
  PartBundleNode(name, new CharacterJointBundle(name)),
  _joints_pcollector(PStatCollector(_animation_pcollector, name), "Joints"),
  _skinning_pcollector(PStatCollector(_animation_pcollector, name), "Vertices")
{
  set_cull_callback();
}

////////////////////////////////////////////////////////////////////
//     Function: Character::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Character::
~Character() {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    r_clear_joint_characters(get_bundle(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::make_copy
//       Access: Public, Virtual
//  Description: The Character make_copy() function will make a new
//               copy of the Character, with all of its joints copied,
//               and with a new set of dynamic vertex arrays all ready
//               to go, but it will not copy any of the original
//               Character's geometry, so the new Character won't look
//               like much.  Use copy_subgraph() to make a full copy
//               of the Character.
////////////////////////////////////////////////////////////////////
PandaNode *Character::
make_copy() const {
  return new Character(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::combine_with
//       Access: Public, Virtual
//  Description: Collapses this node with the other node, if possible,
//               and returns a pointer to the combined node, or NULL
//               if the two nodes cannot safely be combined.
//
//               The return value may be this, other, or a new node
//               altogether.
//
//               This function is called from GraphReducer::flatten(),
//               and need not deal with children; its job is just to
//               decide whether to collapse the two nodes and what the
//               collapsed node should look like.
////////////////////////////////////////////////////////////////////
PandaNode *Character::
combine_with(PandaNode *other) {
  if (is_exact_type(get_class_type()) &&
      other->is_exact_type(get_class_type())) {
    // Two Characters can combine by moving PartBundles from one to the other.
    Character *c_other = DCAST(Character, other);
    steal_bundles(c_other);
    return this;
  }

  return PandaNode::combine_with(other);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::cull_callback
//       Access: Public, Virtual
//  Description: This function will be called during the cull
//               traversal to perform any additional operations that
//               should be performed at cull time.  This may include
//               additional manipulation of render state or additional
//               visible/invisible decisions, or any other arbitrary
//               operation.
//
//               Note that this function will *not* be called unless
//               set_cull_callback() is called in the constructor of
//               the derived class.  It is necessary to call
//               set_cull_callback() to indicated that we require
//               cull_callback() to be called.
//
//               By the time this function is called, the node has
//               already passed the bounding-volume test for the
//               viewing frustum, and the node's transform and state
//               have already been applied to the indicated
//               CullTraverserData object.
//
//               The return value is true if this node should be
//               visible, or false if it should be culled.
////////////////////////////////////////////////////////////////////
bool Character::
cull_callback(CullTraverser *, CullTraverserData &) {
  // For now, we update the character during the cull traversal; this
  // prevents us from needlessly updating characters that aren't in
  // the view frustum.  We may need a better way to do this
  // optimization later, to handle characters that might animate
  // themselves in front of the view frustum.
  update();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::calc_tight_bounds
//       Access: Public, Virtual
//  Description: This is used to support
//               NodePath::calc_tight_bounds().  It is not intended to
//               be called directly, and it has nothing to do with the
//               normal Panda bounding-volume computation.
//
//               If the node contains any geometry, this updates
//               min_point and max_point to enclose its bounding box.
//               found_any is to be set true if the node has any
//               geometry at all, or left alone if it has none.  This
//               method may be called over several nodes, so it may
//               enter with min_point, max_point, and found_any
//               already set.
//
//               This function is recursive, and the return value is
//               the transform after it has been modified by this
//               node's transform.
////////////////////////////////////////////////////////////////////
CPT(TransformState) Character::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  // This method is overridden by Character solely to provide a hook
  // to force the joints to update before computing the bounding
  // volume.
  ((Character *)this)->update_to_now();

  return PandaNode::calc_tight_bounds(min_point, max_point, 
                                      found_any, transform, current_thread);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::find_joint
//       Access: Published
//  Description: Returns a pointer to the joint with the given name,
//               if there is such a joint, or NULL if there is no such
//               joint.  This will not return a pointer to a slider.
////////////////////////////////////////////////////////////////////
CharacterJoint *Character::
find_joint(const string &name) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    PartGroup *part = get_bundle(i)->find_child(name);
    if (part != (PartGroup *)NULL &&
        part->is_of_type(CharacterJoint::get_class_type())) {
      return DCAST(CharacterJoint, part);
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::find_slider
//       Access: Published
//  Description: Returns a pointer to the slider with the given name,
//               if there is such a slider, or NULL if there is no such
//               slider.  This will not return a pointer to a joint.
////////////////////////////////////////////////////////////////////
CharacterSlider *Character::
find_slider(const string &name) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    PartGroup *part = get_bundle(i)->find_child(name);
    if (part != (PartGroup *)NULL &&
        part->is_of_type(CharacterSlider::get_class_type())) {
      return DCAST(CharacterSlider, part);
    }
  }

  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_parts
//       Access: Published
//  Description: Writes a list of the Character's joints and sliders,
//               in their hierchical structure, to the indicated
//               output stream.
////////////////////////////////////////////////////////////////////
void Character::
write_parts(ostream &out) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    get_bundle(i)->write(out, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_part_values
//       Access: Published
//  Description: Writes a list of the Character's joints and sliders,
//               along with each current position, in their hierchical
//               structure, to the indicated output stream.
////////////////////////////////////////////////////////////////////
void Character::
write_part_values(ostream &out) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    get_bundle(i)->write_with_value(out, 0);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::update_to_now
//       Access: Published
//  Description: Advances the character's frame to the current time,
//               and then calls update().  This can be used by show
//               code to force an update of the character's position
//               to the current frame, regardless of whether the
//               character is currently onscreen and animating.
//
//               This method is deprecated.  Call update() instead.
////////////////////////////////////////////////////////////////////
void Character::
update_to_now() {
  update();
}

////////////////////////////////////////////////////////////////////
//     Function: Character::update
//       Access: Published
//  Description: Recalculates the Character's joints and vertices for
//               the current frame.  Normally this is performed
//               automatically during the render and need not be
//               called explicitly.
////////////////////////////////////////////////////////////////////
void Character::
update() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now != _last_auto_update) {
    _last_auto_update = now;

    PStatTimer timer(_joints_pcollector);
    if (char_cat.is_spam()) {
      char_cat.spam() << "Animating " << *this << " at time " << now << "\n";
    }
    
    do_update();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::force_update
//       Access: Published
//  Description: Recalculates the character even if we think it
//               doesn't need it.
////////////////////////////////////////////////////////////////////
void Character::
force_update() {
  // Statistics
  PStatTimer timer(_joints_pcollector);

  // Update all the joints and sliders.
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    get_bundle(i)->force_update();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::do_update
//       Access: Private
//  Description: The actual implementation of update().  Assumes the
//               appropriate PStatCollector has already been started.
////////////////////////////////////////////////////////////////////
void Character::
do_update() {
  // Update all the joints and sliders.
  if (even_animation) {
    int num_bundles = get_num_bundles();
    for (int i = 0; i < num_bundles; ++i) {
      get_bundle(i)->force_update();
    }
  } else {
    int num_bundles = get_num_bundles();
    for (int i = 0; i < num_bundles; ++i) {
      get_bundle(i)->update();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_joints
//       Access: Private
//  Description: Recursively walks the joint/slider hierarchy and
//               creates a new copy of the hierarchy.
////////////////////////////////////////////////////////////////////
void Character::
copy_joints(PartGroup *copy, PartGroup *orig) {
  if (copy->get_type() != orig->get_type()) {
    char_cat.warning()
      << "Don't know how to copy " << orig->get_type() << "\n";
  }

  PartGroup::Children::const_iterator ci;
  for (ci = orig->_children.begin(); ci != orig->_children.end(); ++ci) {
    PartGroup *orig_child = (*ci);
    PartGroup *copy_child = orig_child->make_copy();
    if (copy_child->is_of_type(CharacterJoint::get_class_type())) {
      DCAST(CharacterJoint, copy_child)->set_character(this);
    }
    copy->_children.push_back(copy_child);
    copy_joints(copy_child, orig_child);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::r_copy_children
//       Access: Protected, Virtual
//  Description: This is called by r_copy_subgraph(); the copy has
//               already been made of this particular node (and this
//               is the copy); this function's job is to copy all of
//               the children from the original.
//
//               Note that it includes the parameter inst_map, which
//               is a map type, and is not (and cannot be) exported
//               from PANDA.DLL.  Thus, any derivative of PandaNode
//               that is not also a member of PANDA.DLL *cannot*
//               access this map, and probably should not even
//               override this function.
////////////////////////////////////////////////////////////////////
void Character::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map,
                Thread *current_thread) {
  // We assume there will be no instancing going on below the
  // Character node.  If there is, too bad; it will get flattened out.

  // We preempt the node's r_copy_children() operation with our own
  // function that keeps track of the old vs. new nodes and also
  // updates any Geoms we find with our new dynamic vertices.

  const Character *from_char;
  DCAST_INTO_V(from_char, from);
  NodeMap node_map;
  JointMap joint_map;

  int num_bundles = get_num_bundles();
  nassertv(from_char->get_num_bundles() == num_bundles);
  int i;
  for (i = 0; i < num_bundles; ++i) {
    fill_joint_map(joint_map, get_bundle(i), from_char->get_bundle(i));
  }

  GeomVertexMap gvmap;
  GeomJointMap gjmap;
  GeomSliderMap gsmap;
  r_copy_char(this, from_char, from_char, node_map, joint_map, 
              gvmap, gjmap, gsmap);

  for (i = 0; i < num_bundles; ++i) {
    copy_node_pointers(node_map, get_bundle(i), from_char->get_bundle(i));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::fill_joint_map
//       Access: Private
//  Description: After the joint hierarchy has already been copied
//               from the indicated hierarchy, this recursively walks
//               through the joints and builds up a mapping from old
//               to new.
////////////////////////////////////////////////////////////////////
void Character::
fill_joint_map(Character::JointMap &joint_map, 
               PartGroup *copy, PartGroup *orig) {
  joint_map[orig] = copy;

  int i = 0, j = 0;
  int copy_num_children = copy->get_num_children();
  int orig_num_children = orig->get_num_children();

  while (i < copy_num_children && j < orig_num_children) {
    PartGroup *pc = copy->get_child(i);
    PartGroup *ac = orig->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      i++;
    } else if (ac->get_name() < pc->get_name()) {
      j++;
    } else {
      fill_joint_map(joint_map, pc, ac);
      i++;
      j++;
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Character::r_copy_char
//       Access: Private
//  Description: Recursively walks the scene graph hierarchy below the
//               Character node, duplicating it while noting the
//               orig:copy node mappings, and also updates any
//               GeomNodes found.
////////////////////////////////////////////////////////////////////
void Character::
r_copy_char(PandaNode *dest, const PandaNode *source,
            const Character *from, Character::NodeMap &node_map,
            const Character::JointMap &joint_map,
            Character::GeomVertexMap &gvmap,
            Character::GeomJointMap &gjmap, Character::GeomSliderMap &gsmap) {

  if (source->is_geom_node()) {
    const GeomNode *source_gnode;
    GeomNode *dest_gnode;
    DCAST_INTO_V(source_gnode, source);
    DCAST_INTO_V(dest_gnode, dest);

    dest_gnode->remove_all_geoms();
    int num_geoms = source_gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      const Geom *geom = source_gnode->get_geom(i);
      const RenderState *state = source_gnode->get_geom_state(i);
      dest_gnode->add_geom(copy_geom(geom, from, joint_map, gvmap, gjmap, gsmap), state);
    }
  }

  int num_children = source->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PandaNode *source_child = source->get_child(i);
    int source_sort = source->get_child_sort(i);

    PT(PandaNode) dest_child;
    if (source_child->is_of_type(Character::get_class_type())) {
      // We make a special case for nodes of type Character.  If we
      // encounter one of these, we have a Character under a
      // Character, and the nested Character's copy should be called
      // instead of ours.
      dest_child = source_child->copy_subgraph();

    } else {
      // Otherwise, we assume that make_copy() will make a suitable
      // copy of the node.  This does limit the sorts of things we can
      // have parented to a Character and expect copy_subgraph() to
      // work correctly.  Too bad.
      dest_child = source_child->make_copy();
      r_copy_char(dest_child, source_child, from, node_map, joint_map,
                  gvmap, gjmap, gsmap);
    }
    dest->add_child(dest_child, source_sort);
    node_map[source_child] = dest_child;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_geom
//       Access: Private
//  Description: Makes a new copy of the Geom with the dynamic vertex
//               arrays replaced to reference this Character instead
//               of the other one.  If no arrays have changed, simply
//               returns the same Geom.
////////////////////////////////////////////////////////////////////
PT(Geom) Character::
copy_geom(const Geom *source, const Character *from,
          const Character::JointMap &joint_map,
          Character::GeomVertexMap &gvmap,
          Character::GeomJointMap &gjmap, Character::GeomSliderMap &gsmap) {
  CPT(GeomVertexFormat) format = source->get_vertex_data()->get_format();
  if (format->get_animation().get_animation_type() == Geom::AT_none) {
    // Not animated, so never mind.
    return (Geom *)source;
  }
  
  PT(Geom) dest = source->make_copy();
  
  CPT(GeomVertexData) orig_vdata = source->get_vertex_data();
  PT(GeomVertexData) new_vdata;
  GeomVertexMap::iterator gvmi = gvmap.find(orig_vdata);
  if (gvmi != gvmap.end()) {
    new_vdata = (*gvmi).second;
  } else {
    new_vdata = new GeomVertexData(*orig_vdata);

    new_vdata->set_transform_table(redirect_transform_table(orig_vdata->get_transform_table(), joint_map, gjmap));
    new_vdata->set_transform_blend_table(redirect_transform_blend_table(orig_vdata->get_transform_blend_table(), joint_map, gjmap));
    new_vdata->set_slider_table(redirect_slider_table(orig_vdata->get_slider_table(), gsmap));

    gvmap.insert(GeomVertexMap::value_type(orig_vdata, new_vdata));
  }  

  dest->set_vertex_data(new_vdata);
  
  return dest;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::copy_node_pointers
//       Access: Public
//  Description: Creates _net_transform_nodes and _local_transform_nodes
//               as appropriate in each of the Character's joints, as
//               copied from the other Character.
////////////////////////////////////////////////////////////////////
void Character::
copy_node_pointers(const Character::NodeMap &node_map,
                   PartGroup *dest, const PartGroup *source) {
  if (dest->is_of_type(CharacterJoint::get_class_type())) {
    nassertv(dest != source);
    const CharacterJoint *source_joint;
    CharacterJoint *dest_joint;
    DCAST_INTO_V(source_joint, source);
    DCAST_INTO_V(dest_joint, dest);
    
    CharacterJoint::NodeList::const_iterator ai;
    for (ai = source_joint->_net_transform_nodes.begin();
         ai != source_joint->_net_transform_nodes.end();
         ++ai) {
      PandaNode *source_node = (*ai);
      
      NodeMap::const_iterator mi;
      mi = node_map.find(source_node);
      if (mi != node_map.end()) {
        PandaNode *dest_node = (*mi).second;
        
        // Here's an internal joint that the source Character was
        // animating directly.  We'll animate our corresponding
        // joint the same way.
        dest_joint->set_character(this);
        dest_joint->add_net_transform(dest_node);
      }
    }
    
    for (ai = source_joint->_local_transform_nodes.begin();
         ai != source_joint->_local_transform_nodes.end();
         ++ai) {
      PandaNode *source_node = (*ai);
      
      NodeMap::const_iterator mi;
      mi = node_map.find(source_node);
      if (mi != node_map.end()) {
        PandaNode *dest_node = (*mi).second;
        
        // Here's an internal joint that the source Character was
        // animating directly.  We'll animate our corresponding
        // joint the same way.
        dest_joint->set_character(this);
        dest_joint->add_local_transform(dest_node);
      }
    }
  }

  // Now recurse over children.
  int i = 0, j = 0;
  int dest_num_children = dest->get_num_children();
  int source_num_children = source->get_num_children();

  while (i < dest_num_children && j < source_num_children) {
    PartGroup *pc = dest->get_child(i);
    PartGroup *ac = source->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      i++;
    } else if (ac->get_name() < pc->get_name()) {
      j++;
    } else {
      copy_node_pointers(node_map, pc, ac);
      i++;
      j++;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::redirect_transform_table
//       Access: Private
//  Description: Creates a new TransformTable, similar to the
//               indicated one, with the joint and slider pointers
//               redirected into this object.
////////////////////////////////////////////////////////////////////
CPT(TransformTable) Character::
redirect_transform_table(const TransformTable *source,
                         const Character::JointMap &joint_map,
                         Character::GeomJointMap &gjmap) {
  if (source == (TransformTable *)NULL) {
    return NULL;
  }

  PT(TransformTable) dest = new TransformTable(*source);

  int num_transforms = dest->get_num_transforms();
  for (int i = 0; i < num_transforms; ++i) {
    const VertexTransform *vt = dest->get_transform(i);
    PT(JointVertexTransform) new_jvt = redirect_joint(vt, joint_map, gjmap);
    if (new_jvt != (JointVertexTransform *)NULL) {
      dest->set_transform(i, new_jvt);
    }
  }

  return TransformTable::register_table(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::redirect_transform_blend_table
//       Access: Private
//  Description: Creates a new TransformBlendTable, similar to the
//               indicated one, with the joint and slider pointers
//               redirected into this object.
////////////////////////////////////////////////////////////////////
CPT(TransformBlendTable) Character::
redirect_transform_blend_table(const TransformBlendTable *source,
                               const Character::JointMap &joint_map,
                               Character::GeomJointMap &gjmap) {
  if (source == (TransformBlendTable *)NULL) {
    return NULL;
  }

  PT(TransformBlendTable) dest = new TransformBlendTable(*source);

  int num_blends = dest->get_num_blends();
  for (int i = 0; i < num_blends; ++i) {
    TransformBlend blend = dest->get_blend(i);
    int num_transforms = blend.get_num_transforms();
    for (int j = 0; j < num_transforms; ++j) {
      const VertexTransform *vt = blend.get_transform(j);
      PT(JointVertexTransform) new_jvt = redirect_joint(vt, joint_map, gjmap);
      if (new_jvt != (JointVertexTransform *)NULL) {
        blend.set_transform(j, new_jvt);
      }
    }
    dest->set_blend(i, blend);
  }

  return dest;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::redirect_slider_table
//       Access: Private
//  Description: Creates a new SliderTable, similar to the
//               indicated one, with the joint and slider pointers
//               redirected into this object.
////////////////////////////////////////////////////////////////////
CPT(SliderTable) Character::
redirect_slider_table(const SliderTable *source,
                      Character::GeomSliderMap &gsmap) {
  if (source == (SliderTable *)NULL) {
    return NULL;
  }

  PT(SliderTable) dest = new SliderTable(*source);

  int num_sliders = dest->get_num_sliders();
  for (int i = 0; i < num_sliders; ++i) {
    const VertexSlider *vs = dest->get_slider(i);
    PT(CharacterVertexSlider) new_cvs = redirect_slider(vs, gsmap);
    if (new_cvs != (CharacterVertexSlider *)NULL) {
      dest->set_slider(i, new_cvs);
    }
  }

  return SliderTable::register_table(dest);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::redirect_joint
//       Access: Private
//  Description: Creates a new JointVertexTransform that is similar to
//               the indicated one, but points into this character.
//               If one was already created (in the GeomJointMap), returns
//               it instead.
////////////////////////////////////////////////////////////////////
PT(JointVertexTransform) Character::
redirect_joint(const VertexTransform *vt, 
               const Character::JointMap &joint_map,
               Character::GeomJointMap &gjmap) {
  GeomJointMap::iterator ji;
  ji = gjmap.find(vt);
  if (ji != gjmap.end()) {
    return (*ji).second;
  }

  PT(JointVertexTransform) new_jvt;
  
  if (vt->is_of_type(JointVertexTransform::get_class_type())) {
    const JointVertexTransform *jvt = DCAST(JointVertexTransform, vt);
    const CharacterJoint *orig_joint = jvt->get_joint();
    JointMap::const_iterator jmi = joint_map.find(orig_joint);
    if (jmi == joint_map.end()) {
      char_cat.error()
        << "Could not find joint " << *orig_joint
        << " within the character hierarchy.\n";

    } else {
      CharacterJoint *joint = DCAST(CharacterJoint, (*jmi).second);
      new_jvt = new JointVertexTransform(joint);
    }
  }

  gjmap[vt] = new_jvt;
  return new_jvt;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::redirect_slider
//       Access: Private
//  Description: Creates a new CharacterVertexSlider that is similar to
//               the indicated one, but points into this character.
//               If one was already created (in the GeomSliderMap), returns
//               it instead.
////////////////////////////////////////////////////////////////////
PT(CharacterVertexSlider) Character::
redirect_slider(const VertexSlider *vs, Character::GeomSliderMap &gsmap) {
  GeomSliderMap::iterator ji;
  ji = gsmap.find(vs);
  if (ji != gsmap.end()) {
    return (*ji).second;
  }

  PT(CharacterVertexSlider) new_cvs;

  if (vs->is_of_type(CharacterVertexSlider::get_class_type())) {
    const CharacterVertexSlider *cvs = DCAST(CharacterVertexSlider, vs);
    CharacterSlider *slider = find_slider(cvs->get_char_slider()->get_name());
    if (slider != (CharacterSlider *)NULL) {
      new_cvs = new CharacterVertexSlider(slider);
    }
  }

  gsmap[vs] = new_cvs;
  return new_cvs;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::r_clear_joint_characters
//       Access: Private
//  Description: Recursively walks through the joint hierarchy and
//               clears any _character pointers on all the joints.
//               Intended to be called just before Character
//               destruction.
////////////////////////////////////////////////////////////////////
void Character::
r_clear_joint_characters(PartGroup *part) {
  if (part->is_of_type(CharacterJoint::get_class_type())) {
    CharacterJoint *joint = DCAST(CharacterJoint, part);
    nassertv(joint->get_character() == this || joint->get_character() == NULL);
    joint->set_character(NULL);
  }

  int num_children = part->get_num_children();
  for (int i = 0; i < num_children; ++i) {
    PartGroup *child = part->get_child(i);
    r_clear_joint_characters(child);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Character::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               Character.
////////////////////////////////////////////////////////////////////
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Character::
write_datagram(BamWriter *manager, Datagram &dg) {
  PartBundleNode::write_datagram(manager, dg);

  // Record 0 parts written--we no longer write an array of parts.
  dg.add_uint16(0);
}

////////////////////////////////////////////////////////////////////
//     Function: Character::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int Character::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  // Pretend to read the _temp_num_parts parts that were found in the
  // bam file.
  return PartBundleNode::complete_pointers(p_list, manager) + _temp_num_parts;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type Character is encountered
//               in the Bam file.  It should create the Character
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *Character::
make_from_bam(const FactoryParams &params) {
  Character *node = new Character("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: Character::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Character.
////////////////////////////////////////////////////////////////////
void Character::
fillin(DatagramIterator &scan, BamReader *manager) {
  PartBundleNode::fillin(scan, manager);

  // We no longer read an array of parts here, but for backward
  // compatibility, we must read in the number of parts that used to
  // be there, and read past each of the pointers.
  _temp_num_parts = scan.get_uint16();
  for (unsigned int i = 0; i < _temp_num_parts; i++) {
    manager->read_pointer(scan);
  }

#ifdef DO_PSTATS
  // Reinitialize our collectors with our name, now that we know it.
  if (has_name()) {
    _joints_pcollector = 
      PStatCollector(PStatCollector(_animation_pcollector, get_name()), "Joints");
    _skinning_pcollector = 
      PStatCollector(PStatCollector(_animation_pcollector, get_name()), "Vertices");
  }
#endif
}
