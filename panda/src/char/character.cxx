/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file character.cxx
 * @author drose
 * @date 2002-03-06
 */

#include "character.h"
#include "characterJoint.h"
#include "config_char.h"
#include "nodePath.h"
#include "geomNode.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "pStatTimer.h"
#include "animControl.h"
#include "clockObject.h"
#include "pStatTimer.h"
#include "camera.h"
#include "cullTraverser.h"
#include "cullTraverserData.h"

TypeHandle Character::_type_handle;

PStatCollector Character::_animation_pcollector("*:Animation");

/**
 * Use make_copy() or copy_subgraph() to copy a Character.
 */
Character::
Character(const Character &copy, bool copy_bundles) :
  PartBundleNode(copy),
  _lod_center(copy._lod_center),
  _lod_far_distance(copy._lod_far_distance),
  _lod_near_distance(copy._lod_near_distance),
  _lod_delay_factor(copy._lod_delay_factor),
  _do_lod_animation(copy._do_lod_animation),
  _joints_pcollector(copy._joints_pcollector),
  _skinning_pcollector(copy._skinning_pcollector)
{
  set_cull_callback();

  if (copy_bundles) {
    // Copy the bundle(s).
    int num_bundles = copy.get_num_bundles();
    for (int i = 0; i < num_bundles; ++i) {
      PartBundle *orig_bundle = copy.get_bundle(i);
      PT(PartBundle) new_bundle = DCAST(PartBundle, orig_bundle->copy_subgraph());
      add_bundle(new_bundle);
    }
  } else {
    // Share the bundles.
    int num_bundles = copy.get_num_bundles();
    for (int i = 0; i < num_bundles; ++i) {
      PartBundle *orig_bundle = copy.get_bundle(i);
      add_bundle(orig_bundle);
    }
  }
  _last_auto_update = -1.0;
  _view_frame = -1;
  _view_distance2 = 0.0f;
}

/**
 *
 */
Character::
Character(const std::string &name) :
  PartBundleNode(name, new CharacterJointBundle(name)),
  _joints_pcollector(PStatCollector(_animation_pcollector, name), "Joints"),
  _skinning_pcollector(PStatCollector(_animation_pcollector, name), "Vertices")
{
  set_cull_callback();
  clear_lod_animation();
  _last_auto_update = -1.0;
  _view_frame = -1;
  _view_distance2 = 0.0f;
}

/**
 *
 */
Character::
~Character() {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    r_clear_joint_characters(get_bundle(i));
  }
}

/**
 * The Character make_copy() function will make a new copy of the Character,
 * with all of its joints copied, and with a new set of dynamic vertex arrays
 * all ready to go, but it will not copy any of the original Character's
 * geometry, so the new Character won't look like much.  Use copy_subgraph()
 * to make a full copy of the Character.
 */
PandaNode *Character::
make_copy() const {
  return new Character(*this, true);
}

/**
 * This is similar to make_copy(), but it makes a copy for the specific
 * purpose of flatten.  Typically, this will be a new PandaNode with a new
 * pointer, but all of the internal data will always be shared with the
 * original; whereas the new node returned by make_copy() might not share the
 * internal data.
 */
PandaNode *Character::
dupe_for_flatten() const {
  return new Character(*this, false);
}

/**
 * Collapses this node with the other node, if possible, and returns a pointer
 * to the combined node, or NULL if the two nodes cannot safely be combined.
 *
 * The return value may be this, other, or a new node altogether.
 *
 * This function is called from GraphReducer::flatten(), and need not deal
 * with children; its job is just to decide whether to collapse the two nodes
 * and what the collapsed node should look like.
 */
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

/**
 * This function will be called during the cull traversal to perform any
 * additional operations that should be performed at cull time.  This may
 * include additional manipulation of render state or additional
 * visible/invisible decisions, or any other arbitrary operation.
 *
 * Note that this function will *not* be called unless set_cull_callback() is
 * called in the constructor of the derived class.  It is necessary to call
 * set_cull_callback() to indicated that we require cull_callback() to be
 * called.
 *
 * By the time this function is called, the node has already passed the
 * bounding-volume test for the viewing frustum, and the node's transform and
 * state have already been applied to the indicated CullTraverserData object.
 *
 * The return value is true if this node should be visible, or false if it
 * should be culled.
 */
bool Character::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  // For now, we update the character during the cull traversal; this prevents
  // us from needlessly updating characters that aren't in the view frustum.
  // We may need a better way to do this optimization later, to handle
  // characters that might animate themselves in front of the view frustum.

  if (_do_lod_animation) {
    int this_frame = ClockObject::get_global_clock()->get_frame_count();

    CPT(TransformState) rel_transform = get_rel_transform(trav, data);
    LPoint3 center = _lod_center * rel_transform->get_mat();
    PN_stdfloat dist2 = center.dot(center);

    if (this_frame != _view_frame || dist2 < _view_distance2) {
      _view_frame = this_frame;
      _view_distance2 = dist2;

      // Now compute the lod delay.
      PN_stdfloat dist = sqrt(dist2);
      double delay = 0.0;
      if (dist > _lod_near_distance) {
        delay = _lod_delay_factor * (dist - _lod_near_distance) / (_lod_far_distance - _lod_near_distance);
        nassertr(delay > 0.0, false);
      }
      set_lod_current_delay(delay);

      if (char_cat.is_spam()) {
        char_cat.spam()
          << "Distance to " << NodePath::any_path(this) << " in frame "
          << this_frame << " is " << dist << ", computed delay is " << delay
          << "\n";
      }
    }
  }

  update();
  return true;
}

/**
 * This is used to support NodePath::calc_tight_bounds().  It is not intended
 * to be called directly, and it has nothing to do with the normal Panda
 * bounding-volume computation.
 *
 * If the node contains any geometry, this updates min_point and max_point to
 * enclose its bounding box.  found_any is to be set true if the node has any
 * geometry at all, or left alone if it has none.  This method may be called
 * over several nodes, so it may enter with min_point, max_point, and
 * found_any already set.
 *
 * This function is recursive, and the return value is the transform after it
 * has been modified by this node's transform.
 */
CPT(TransformState) Character::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point, bool &found_any,
                  const TransformState *transform, Thread *current_thread) const {
  // This method is overridden by Character solely to provide a hook to force
  // the joints to update before computing the bounding volume.
  ((Character *)this)->update_to_now();

  // Unfortunately, calling update_to_now() will invalidate the node's cached
  // bounding volume, which causes a problem when this is called during the
  // traversal, e.g.  due to a ShowBoundsEffect.  As a hacky fix to work
  // around this, we will force-recompute all of the bounding volumes of our
  // parent nodes immediately.
  Parents parents = get_parents();
  for (size_t i = 0; i < parents.get_num_parents(); ++i) {
    PandaNode *parent = parents.get_parent(i);
    parent->get_bounds();
  }

  return PandaNode::calc_tight_bounds(min_point, max_point,
                                      found_any, transform, current_thread);
}

/**
 * Merges old_bundle with new_bundle.  old_bundle must be one of the
 * PartBundles within this node.  At the end of this call, the old_bundle
 * pointer within this node will be replaced with the new_bundle pointer, and
 * all geometry within this node will be updated to reference new_bundle.
 *
 * @deprecated Use the newer version of this method, below.
 */
void Character::
merge_bundles(PartBundle *old_bundle, PartBundle *new_bundle) {
  if (old_bundle == new_bundle) {
    // Trivially return.
    return;
  }

  // Find the PartBundleHandle of old_bundle.
  PT(PartBundleHandle) old_bundle_handle;
  Bundles::const_iterator bi;
  for (bi = _bundles.begin(); bi != _bundles.end(); ++bi) {
    if ((*bi)->get_bundle() == old_bundle) {
      old_bundle_handle = (*bi);
      break;
    }
  }
  nassertv(!old_bundle_handle.is_null());

  PT(PartBundleHandle) new_bundle_handle = new PartBundleHandle(new_bundle);
  merge_bundles(old_bundle_handle, new_bundle_handle);
}

/**
 * Merges old_bundle_handle->get_bundle() with new_bundle.  old_bundle_handle
 * must be one of the PartBundleHandle within this node.  At the end of this
 * call, the bundle pointer within the old_bundle_handle will be replaced with
 * that within the new_bundle_handle pointer, and all geometry within this
 * node will be updated to reference new_bundle.
 *
 * Normally, this is called when the two bundles have the same, or nearly the
 * same, hierarchies.  In this case, new_bundle will simply be assigned over
 * the old_bundle position.  However, if any joints are present in one bundle
 * or the other, new_bundle will be modified to contain the union of all
 * joints.
 *
 * The geometry below this node is also updated to reference new_bundle,
 * instead of the original old_bundle.
 *
 * This method is intended to unify two different models that share a common
 * skeleton, for instance, different LOD's of the same model.
 */
void Character::
merge_bundles(PartBundleHandle *old_bundle_handle,
              PartBundleHandle *new_bundle_handle) {
  PartBundle *old_bundle = old_bundle_handle->get_bundle();
  PartBundle *new_bundle = new_bundle_handle->get_bundle();
  new_bundle->merge_anim_preloads(old_bundle);

  update_bundle(old_bundle_handle, new_bundle);
}

/**
 * Activates a special mode in which the character animates less frequently as
 * it gets further from the camera.  This is intended as a simple optimization
 * to minimize the effort of computing animation for lots of characters that
 * may not necessarily be very important to animate every frame.
 *
 * If the character is closer to the camera than near_distance, then it is
 * animated its normal rate, every frame.  If the character is exactly
 * far_distance away, it is animated only every delay_factor seconds (which
 * should be a number greater than 0).  If the character is between
 * near_distance and far_distance, its animation rate is linearly interpolated
 * according to its distance between the two.  The interpolation function
 * continues beyond far_distance, so that the character is animated
 * increasingly less frequently as it gets farther away.
 *
 * The distance calculations are made from center, which is a fixed point
 * relative to the character node, to the camera's lod center or cull center
 * node (or to the camera node itself).
 *
 * If multiple cameras are viewing the character in any given frame, the
 * closest one counts.
 */
void Character::
set_lod_animation(const LPoint3 &center,
                  PN_stdfloat far_distance, PN_stdfloat near_distance,
                  PN_stdfloat delay_factor) {
  nassertv(far_distance >= near_distance);
  nassertv(delay_factor >= 0.0f);
  _lod_center = center;
  _lod_far_distance = far_distance;
  _lod_near_distance = near_distance;
  _lod_delay_factor = delay_factor;
  _do_lod_animation = (_lod_far_distance > _lod_near_distance && _lod_delay_factor > 0.0);
  if (!_do_lod_animation) {
    set_lod_current_delay(0.0);
  }
}

/**
 * Undoes the effect of a recent call to set_lod_animation().  Henceforth, the
 * character will animate every frame, regardless of its distance from the
 * camera.
 */
void Character::
clear_lod_animation() {
  _lod_center = LPoint3::zero();
  _lod_far_distance = 0.0f;
  _lod_near_distance = 0.0f;
  _lod_delay_factor = 0.0f;
  _do_lod_animation = false;
  set_lod_current_delay(0.0);
}

/**
 * Returns a pointer to the joint with the given name, if there is such a
 * joint, or NULL if there is no such joint.  This will not return a pointer
 * to a slider.
 */
CharacterJoint *Character::
find_joint(const std::string &name) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    PartGroup *part = get_bundle(i)->find_child(name);
    if (part != nullptr && part->is_character_joint()) {
      return DCAST(CharacterJoint, part);
    }
  }

  return nullptr;
}

/**
 * Returns a pointer to the slider with the given name, if there is such a
 * slider, or NULL if there is no such slider.  This will not return a pointer
 * to a joint.
 */
CharacterSlider *Character::
find_slider(const std::string &name) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    PartGroup *part = get_bundle(i)->find_child(name);
    if (part != nullptr &&
        part->is_of_type(CharacterSlider::get_class_type())) {
      return DCAST(CharacterSlider, part);
    }
  }

  return nullptr;
}

/**
 * Writes a list of the Character's joints and sliders, in their hierchical
 * structure, to the indicated output stream.
 */
void Character::
write_parts(std::ostream &out) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    get_bundle(i)->write(out, 0);
  }
}

/**
 * Writes a list of the Character's joints and sliders, along with each
 * current position, in their hierchical structure, to the indicated output
 * stream.
 */
void Character::
write_part_values(std::ostream &out) const {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    get_bundle(i)->write_with_value(out, 0);
  }
}

/**
 * Advances the character's frame to the current time, and then calls
 * update().  This can be used by show code to force an update of the
 * character's position to the current frame, regardless of whether the
 * character is currently onscreen and animating.
 *
 * @deprecated Call update() instead.
 */
void Character::
update_to_now() {
  update();
}

/**
 * Recalculates the Character's joints and vertices for the current frame.
 * Normally this is performed automatically during the render and need not be
 * called explicitly.
 */
void Character::
update() {
  double now = ClockObject::get_global_clock()->get_frame_time();
  if (now != _last_auto_update) {
    _last_auto_update = now;

    if (char_cat.is_spam()) {
      char_cat.spam()
        << "Animating " << NodePath::any_path(this)
        << " at time " << now << "\n";
    }

    PStatTimer timer(_joints_pcollector);
    do_update();
  }
}

/**
 * Recalculates the character even if we think it doesn't need it.
 */
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

/**
 * This is called by r_copy_subgraph(); the copy has already been made of this
 * particular node (and this is the copy); this function's job is to copy all
 * of the children from the original.
 *
 * Note that it includes the parameter inst_map, which is a map type, and is
 * not (and cannot be) exported from PANDA.DLL.  Thus, any derivative of
 * PandaNode that is not also a member of PANDA.DLL *cannot* access this map,
 * and probably should not even override this function.
 */
void Character::
r_copy_children(const PandaNode *from, PandaNode::InstanceMap &inst_map,
                Thread *current_thread) {
  // We assume there will be no instancing going on below the Character node.
  // If there is, too bad; it will get flattened out.

  // We preempt the node's r_copy_children() operation with our own function
  // that keeps track of the old vs.  new nodes and also updates any Geoms we
  // find with our new dynamic vertices.

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

/**
 * Replaces the contents of the indicated PartBundleHandle (presumably stored
 * within this node) with new_bundle.
 */
void Character::
update_bundle(PartBundleHandle *old_bundle_handle, PartBundle *new_bundle) {
  if (old_bundle_handle->get_bundle() == new_bundle) {
    // Trivially return.
    return;
  }

  // First, merge the bundles, to ensure we have the same set of joints in the
  // new bundle.
  JointMap joint_map;
  r_merge_bundles(joint_map, old_bundle_handle->get_bundle(), new_bundle);

  PartBundleNode::update_bundle(old_bundle_handle, new_bundle);

  // Now convert the geometry to use the new bundle.
  GeomVertexMap gvmap;
  GeomJointMap gjmap;
  GeomSliderMap gsmap;
  r_update_geom(this, joint_map, gvmap, gjmap, gsmap);
}

/**
 * Returns the relative transform to convert from the LODNode space to the
 * camera space.
 */
CPT(TransformState) Character::
get_rel_transform(CullTraverser *trav, CullTraverserData &data) {
  // Get a pointer to the camera node.
  Camera *camera = trav->get_scene()->get_camera_node();

  // Get the camera space transform.
  CPT(TransformState) rel_transform;

  NodePath lod_center = camera->get_lod_center();
  if (!lod_center.is_empty()) {
    rel_transform =
      lod_center.get_net_transform()->invert_compose(data.get_net_transform(trav));
  } else {
    NodePath cull_center = camera->get_cull_center();
    if (!cull_center.is_empty()) {
      rel_transform =
        cull_center.get_net_transform()->invert_compose(data.get_net_transform(trav));
    } else {
      rel_transform = data.get_modelview_transform(trav);
    }
  }

  return rel_transform;
}

/**
 * The actual implementation of update().  Assumes the appropriate
 * PStatCollector has already been started.
 */
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

/**
 * Changes the amount of delay we should impose due to the LOD animation
 * setting.
 */
void Character::
set_lod_current_delay(double delay) {
  int num_bundles = get_num_bundles();
  for (int i = 0; i < num_bundles; ++i) {
    get_bundle(i)->set_update_delay(delay);
  }
}

/**
 * After the joint hierarchy has already been copied from the indicated
 * hierarchy, this recursively walks through the joints and builds up a
 * mapping from old to new.
 */
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

/**
 * Recursively checks the two bundles for a matching hierarchy, and adds nodes
 * as necessary to "new_group" where they are not already present.  Also fills
 * joint_map in the same manner as fill_joint_map().
 */
void Character::
r_merge_bundles(Character::JointMap &joint_map,
                PartGroup *old_group, PartGroup *new_group) {
  joint_map[old_group] = new_group;

  if (new_group->is_character_joint()) {
    CharacterJoint *new_joint;
    DCAST_INTO_V(new_joint, new_group);

    // Make sure the new_joint references this as its new character.
    new_joint->_character = this;

    if (old_group != new_group &&
        old_group->is_character_joint()) {
      CharacterJoint *old_joint;
      DCAST_INTO_V(old_joint, old_group);

      // Since the old_joint will be getting dropped, reset its character
      // reference.
      old_joint->_character = nullptr;

      // Copy any _net_transform and _local_transform operations to the new
      // joint.
      CharacterJoint::NodeList::iterator ni;
      for (ni = old_joint->_net_transform_nodes.begin();
           ni != old_joint->_net_transform_nodes.end();
           ++ni) {
        new_joint->_net_transform_nodes.insert(*ni);
      }
      for (ni = old_joint->_local_transform_nodes.begin();
           ni != old_joint->_local_transform_nodes.end();
           ++ni) {
        new_joint->_local_transform_nodes.insert(*ni);
      }
    }
  }

  if (old_group == new_group) {
    return;
  }

  int i = 0, j = 0;
  int old_num_children = old_group->get_num_children();
  int new_num_children = new_group->get_num_children();

  PartGroup::Children new_children(PartGroup::get_class_type());
  new_children.reserve(std::max(old_num_children, new_num_children));

  while (i < old_num_children && j < new_num_children) {
    PartGroup *pc = old_group->get_child(i);
    PartGroup *ac = new_group->get_child(j);

    if (pc->get_name() < ac->get_name()) {
      // Here is a group that exists in old_group, but not in new_group.
      // Duplicate it.
      PartGroup *new_pc = pc->make_copy();
      new_children.push_back(new_pc);

      r_merge_bundles(joint_map, pc, new_pc);
      i++;

    } else if (ac->get_name() < pc->get_name()) {
      // Here is a group that exists in new_group, but not in old_group.
      // Preserve it.
      new_children.push_back(ac);

      r_merge_bundles(joint_map, ac, ac);
      j++;

    } else {
      // Here is a group that exists in both.  Preserve it.
      new_children.push_back(ac);

      r_merge_bundles(joint_map, pc, ac);
      i++;
      j++;
    }
  }

  while (i < old_num_children) {
    PartGroup *pc = old_group->get_child(i);

    // Here is a group that exists in old_group, but not in new_group.
    // Duplicate it.
    PartGroup *new_pc = pc->make_copy();
    new_children.push_back(new_pc);

    r_merge_bundles(joint_map, pc, new_pc);
    i++;
  }

  while (j < new_num_children) {
    PartGroup *ac = new_group->get_child(j);

    // Here is a group that exists in new_group, but not in old_group.
    // Preserve it.
    new_children.push_back(ac);

    r_merge_bundles(joint_map, ac, ac);
    j++;
  }

  new_group->_children.swap(new_children);
}


/**
 * Recursively walks the scene graph hierarchy below the Character node,
 * duplicating it while noting the orig:copy node mappings, and also updates
 * any GeomNodes found.
 */
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
      dest_gnode->add_geom(copy_geom(geom, joint_map, gvmap, gjmap, gsmap), state);
    }
  }

  int num_children = source->get_num_children();
  for (int i = 0; i < num_children; i++) {
    const PandaNode *source_child = source->get_child(i);
    int source_sort = source->get_child_sort(i);

    PT(PandaNode) dest_child;
    if (source_child->is_of_type(Character::get_class_type())) {
      // We make a special case for nodes of type Character.  If we encounter
      // one of these, we have a Character under a Character, and the nested
      // Character's copy should be called instead of ours.
      dest_child = source_child->copy_subgraph();

    } else {
      // Otherwise, we assume that make_copy() will make a suitable copy of
      // the node.  This does limit the sorts of things we can have parented
      // to a Character and expect copy_subgraph() to work correctly.  Too
      // bad.
      dest_child = source_child->make_copy();
      r_copy_char(dest_child, source_child, from, node_map, joint_map,
                  gvmap, gjmap, gsmap);
    }
    dest->add_child(dest_child, source_sort);
    node_map[source_child] = dest_child;
  }
}

/**
 * Walks the hierarchy, updating any GeomNodes in-place to reference the new
 * animation tables within this Character.
 */
void Character::
r_update_geom(PandaNode *node, const Character::JointMap &joint_map,
              Character::GeomVertexMap &gvmap,
              Character::GeomJointMap &gjmap,
              Character::GeomSliderMap &gsmap) {
  if (node->is_geom_node()) {
    GeomNode *gnode;
    DCAST_INTO_V(gnode, node);

    int num_geoms = gnode->get_num_geoms();
    for (int i = 0; i < num_geoms; i++) {
      CPT(Geom) geom = gnode->get_geom(i);
      PT(Geom) new_geom = copy_geom(geom, joint_map, gvmap, gjmap, gsmap);
      gnode->set_geom(i, new_geom);
    }
  }

  int num_children = node->get_num_children();
  for (int i = 0; i < num_children; i++) {
    PandaNode *child = node->get_child(i);

    r_update_geom(child, joint_map, gvmap, gjmap, gsmap);
  }
}

/**
 * Makes a new copy of the Geom with the dynamic vertex arrays replaced to
 * reference this Character instead of the other one.  If no arrays have
 * changed, simply returns the same Geom.
 */
PT(Geom) Character::
copy_geom(const Geom *source, const Character::JointMap &joint_map,
          Character::GeomVertexMap &gvmap, Character::GeomJointMap &gjmap,
          Character::GeomSliderMap &gsmap) {
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

/**
 * Creates _net_transform_nodes and _local_transform_nodes as appropriate in
 * each of the Character's joints, as copied from the other Character.
 */
void Character::
copy_node_pointers(const Character::NodeMap &node_map,
                   PartGroup *dest, const PartGroup *source) {
  if (dest->is_character_joint()) {
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

        // Here's an internal joint that the source Character was animating
        // directly.  We'll animate our corresponding joint the same way.
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

        // Here's an internal joint that the source Character was animating
        // directly.  We'll animate our corresponding joint the same way.
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

/**
 * Creates a new TransformTable, similar to the indicated one, with the joint
 * and slider pointers redirected into this object.
 */
CPT(TransformTable) Character::
redirect_transform_table(const TransformTable *source,
                         const Character::JointMap &joint_map,
                         Character::GeomJointMap &gjmap) {
  if (source == nullptr) {
    return nullptr;
  }

  PT(TransformTable) dest = new TransformTable(*source);

  int num_transforms = dest->get_num_transforms();
  for (int i = 0; i < num_transforms; ++i) {
    const VertexTransform *vt = dest->get_transform(i);
    PT(JointVertexTransform) new_jvt = redirect_joint(vt, joint_map, gjmap);
    if (new_jvt != nullptr) {
      dest->set_transform(i, new_jvt);
    }
  }

  return TransformTable::register_table(dest);
}

/**
 * Creates a new TransformBlendTable, similar to the indicated one, with the
 * joint and slider pointers redirected into this object.
 */
CPT(TransformBlendTable) Character::
redirect_transform_blend_table(const TransformBlendTable *source,
                               const Character::JointMap &joint_map,
                               Character::GeomJointMap &gjmap) {
  if (source == nullptr) {
    return nullptr;
  }

  PT(TransformBlendTable) dest = new TransformBlendTable(*source);

  int num_blends = dest->get_num_blends();
  for (int i = 0; i < num_blends; ++i) {
    TransformBlend blend = dest->get_blend(i);
    int num_transforms = blend.get_num_transforms();
    for (int j = 0; j < num_transforms; ++j) {
      const VertexTransform *vt = blend.get_transform(j);
      PT(JointVertexTransform) new_jvt = redirect_joint(vt, joint_map, gjmap);
      if (new_jvt != nullptr) {
        blend.set_transform(j, new_jvt);
      }
    }
    dest->set_blend(i, blend);
  }

  return dest;
}

/**
 * Creates a new SliderTable, similar to the indicated one, with the joint and
 * slider pointers redirected into this object.
 */
CPT(SliderTable) Character::
redirect_slider_table(const SliderTable *source,
                      Character::GeomSliderMap &gsmap) {
  if (source == nullptr) {
    return nullptr;
  }

  PT(SliderTable) dest = new SliderTable(*source);

  int num_sliders = dest->get_num_sliders();
  for (int i = 0; i < num_sliders; ++i) {
    const VertexSlider *vs = dest->get_slider(i);
    PT(CharacterVertexSlider) new_cvs = redirect_slider(vs, gsmap);
    if (new_cvs != nullptr) {
      dest->set_slider(i, new_cvs);
    }
  }

  return SliderTable::register_table(dest);
}

/**
 * Creates a new JointVertexTransform that is similar to the indicated one,
 * but points into this character.  If one was already created (in the
 * GeomJointMap), returns it instead.
 */
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

/**
 * Creates a new CharacterVertexSlider that is similar to the indicated one,
 * but points into this character.  If one was already created (in the
 * GeomSliderMap), returns it instead.
 */
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
    if (slider != nullptr) {
      new_cvs = new CharacterVertexSlider(slider);
    }
  }

  gsmap[vs] = new_cvs;
  return new_cvs;
}

/**
 * Recursively walks through the joint hierarchy and clears any _character
 * pointers on all the joints.  Intended to be called just before Character
 * destruction.
 */
void Character::
r_clear_joint_characters(PartGroup *part) {
  if (part->is_character_joint()) {
    CharacterJoint *joint = DCAST(CharacterJoint, part);

    // It is possible for the joint to reference a different Character here--
    // after merge_bundles() has been called, a particular joint will be
    // listed within more than one Character node, but it can only point back
    // to one of them.
    if (joint->get_character() == this) {
      joint->set_character(nullptr);
    }
  }

  int num_children = part->get_num_children();
  for (int i = 0; i < num_children; ++i) {
    PartGroup *child = part->get_child(i);
    r_clear_joint_characters(child);
  }
}

/**
 * Tells the BamReader how to create objects of type Character.
 */
void Character::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void Character::
write_datagram(BamWriter *manager, Datagram &dg) {
  PartBundleNode::write_datagram(manager, dg);

  // Record 0 parts written--we no longer write an array of parts.
  dg.add_uint16(0);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int Character::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  // Pretend to read the _temp_num_parts parts that were found in the bam
  // file.
  return PartBundleNode::complete_pointers(p_list, manager) + _temp_num_parts;
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type Character is encountered in the Bam file.  It should create the
 * Character and extract its information from the file.
 */
TypedWritable *Character::
make_from_bam(const FactoryParams &params) {
  Character *node = new Character("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Character.
 */
void Character::
fillin(DatagramIterator &scan, BamReader *manager) {
  PartBundleNode::fillin(scan, manager);

  // We no longer read an array of parts here, but for backward compatibility,
  // we must read in the number of parts that used to be there, and read past
  // each of the pointers.
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
