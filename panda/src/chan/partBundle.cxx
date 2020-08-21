/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file partBundle.cxx
 * @author drose
 * @date 1999-02-22
 */

#include "partBundle.h"
#include "animBundle.h"
#include "animBundleNode.h"
#include "animControl.h"
#include "loader.h"
#include "animPreloadTable.h"
#include "config_chan.h"
#include "bitArray.h"
#include "string_utils.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "configVariableEnum.h"
#include "loaderOptions.h"
#include "bindAnimRequest.h"

#include <algorithm>

using std::istream;
using std::ostream;
using std::string;

TypeHandle PartBundle::_type_handle;


static ConfigVariableEnum<PartBundle::BlendType> anim_blend_type
("anim-blend-type", PartBundle::BT_normalized_linear,
 PRC_DESC("The default blend type to use for blending animations between "
          "frames, or between multiple animations.  See interpolate-frames, "
          "and also PartBundle::set_anim_blend_flag() and "
          "PartBundle::set_frame_blend_flag()."));


/**
 * Normally, you'd use make_copy() or copy_subgraph() to make a copy of this.
 */
PartBundle::
PartBundle(const PartBundle &copy) :
  PartGroup(copy)
{
  _anim_preload = copy._anim_preload;
  _update_delay = 0.0;

  CDWriter cdata(_cycler, true);
  CDReader cdata_from(copy._cycler);
  cdata->_blend_type = cdata_from->_blend_type;
  cdata->_anim_blend_flag = cdata_from->_anim_blend_flag;
  cdata->_frame_blend_flag = cdata_from->_frame_blend_flag;
  cdata->_root_xform = cdata_from->_root_xform;
}

/**
 * Normally, a PartBundle constructor should not be called directly--it will
 * get created when a PartBundleNode is created.
 */
PartBundle::
PartBundle(const string &name) :
  PartGroup(name)
{
  _update_delay = 0.0;
}

/**
 * Allocates and returns a new copy of the node.  Children are not copied, but
 * see copy_subgraph().
 */
PartGroup *PartBundle::
make_copy() const {
  return new PartBundle(*this);
}

/**
 * Copies the contents of the other PartBundle's preload table into this one.
 */
void PartBundle::
merge_anim_preloads(const PartBundle *other) {
  if (other->_anim_preload == nullptr ||
      _anim_preload == other->_anim_preload) {
    // No-op.
    return;
  }

  if (_anim_preload == nullptr) {
    // Trivial case.
    _anim_preload = other->_anim_preload;
    return;
  }

  // Copy-on-write.
  PT(AnimPreloadTable) anim_preload = _anim_preload.get_write_pointer();
  anim_preload->add_anims_from(other->_anim_preload.get_read_pointer());
}

/**
 * Defines the way the character responds to multiple calls to
 * set_control_effect()).  By default, this flag is set false, which disallows
 * multiple animations.  When this flag is false, it is not necessary to
 * explicitly set the control_effect when starting an animation; starting the
 * animation will implicitly remove the control_effect from the previous
 * animation and set it on the current one.
 *
 * However, if this flag is set true, the control_effect must be explicitly
 * set via set_control_effect() whenever an animation is to affect the
 * character.
 */
void PartBundle::
set_anim_blend_flag(bool anim_blend_flag) {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDLockedReader cdata(_cycler);
  if (cdata->_anim_blend_flag != anim_blend_flag) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_anim_blend_flag = anim_blend_flag;

    if (!anim_blend_flag && cdataw->_blend.size() > 1) {
      // If we just changed to disallow animation blending, we should
      // eliminate all the AnimControls other than the most-recently-added
      // one.

      nassertv(cdataw->_last_control_set != nullptr);
      clear_and_stop_intersecting(cdataw->_last_control_set, cdataw);
    }

    cdataw->_anim_changed = true;
  }
}

/**
 * Returns a PartBundle that is a duplicate of this one, but with the
 * indicated transform applied.  If this is called multiple times with the
 * same TransformState pointer, it returns the same PartBundle each time.
 */
PT(PartBundle) PartBundle::
apply_transform(const TransformState *transform) {
  if (transform->is_identity()) {
    // Trivial no-op.
    return this;
  }

  AppliedTransforms::iterator ati = _applied_transforms.find(transform);
  if (ati != _applied_transforms.end()) {
    if ((*ati).first.is_valid_pointer()) {
      if (auto new_bundle = (*ati).second.lock()) {
        // Here's our cached result.
        return new_bundle;
      }
    }
  }

  PT(PartBundle) new_bundle = DCAST(PartBundle, copy_subgraph());
  new_bundle->xform(transform->get_mat());

  if (ati != _applied_transforms.end()) {
    // A stale pointer to a deleted result.  Update it.
    (*ati).first.refresh();
    (*ati).second = new_bundle;
  } else {
    // No such result yet.  Store it.
    bool inserted = _applied_transforms.insert(AppliedTransforms::value_type(transform, new_bundle)).second;
    nassertr(inserted, new_bundle);
  }

  // Make sure the new transform gets immediately applied to all of the
  // joints.
  new_bundle->force_update();

  return new_bundle;
}

/**
 * Sets the control effect of all AnimControls to zero (but does not "stop"
 * the AnimControls).  The character will no longer be affected by any
 * animation, and will return to its default pose (unless restore-initial-pose
 * is false).
 *
 * The AnimControls which are no longer associated will not be using any CPU
 * cycles, but they may still be in the "playing" state; if they are later
 * reassociated with the PartBundle they will resume at their current frame as
 * if they'd been running all along.
 */
void PartBundle::
clear_control_effects() {
  nassertv(Thread::get_current_pipeline_stage() == 0);

  CDLockedReader cdata(_cycler);
  if (!cdata->_blend.empty()) {
    CDWriter cdataw(_cycler, cdata);
    cdataw->_blend.clear();
    cdataw->_anim_changed = true;
    determine_effective_channels(cdataw);
  }
}

/**
 * Writes a one-line description of the bundle.
 */
void PartBundle::
output(ostream &out) const {
  out << get_type() << " " << get_name();
}

/**
 * Writes a brief description of the bundle and all of its descendants.
 */
void PartBundle::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " {\n";
  write_descendants(out, indent_level + 2);
  indent(out, indent_level) << "}\n";
}


/**
 * Binds the animation to the bundle, if possible, and returns a new
 * AnimControl that can be used to start and stop the animation.  If the anim
 * hierarchy does not match the part hierarchy, returns NULL.
 *
 * If hierarchy_match_flags is 0, only an exact match is accepted; otherwise,
 * it may contain a union of PartGroup::HierarchyMatchFlags values indicating
 * conditions that will be tolerated (but warnings will still be issued).
 *
 * If subset is specified, it restricts the binding only to the named subtree
 * of joints.
 *
 * The AnimControl is not stored within the PartBundle; it is the user's
 * responsibility to maintain the pointer.  The animation will automatically
 * unbind itself when the AnimControl destructs (i.e.  its reference count
 * goes to zero).
 */
PT(AnimControl) PartBundle::
bind_anim(AnimBundle *anim, int hierarchy_match_flags,
          const PartSubset &subset) {
  PT(AnimControl) control = new AnimControl(anim->get_name(), this, 1.0f, 0);
  if (do_bind_anim(control, anim, hierarchy_match_flags, subset)) {
    return control;
  }

  return nullptr;
}

/**
 * Binds an animation to the bundle.  The animation is loaded from the disk
 * via the indicated Loader object.  In other respects, this behaves similarly
 * to bind_anim(), with the addition of asynchronous support.
 *
 * If allow_aysnc is true, the load will be asynchronous if possible.  This
 * requires that the animation basename can be found in the PartBundle's
 * preload table (see get_anim_preload()).
 *
 * In an asynchronous load, the animation file will be loaded and bound in a
 * sub-thread.  This means that the animation will not necessarily be
 * available at the time this method returns.  You may still use the returned
 * AnimControl immediately, though, but no visible effect will occur until the
 * animation eventually becomes available.
 *
 * You can test AnimControl::is_pending() to see if the animation has been
 * loaded yet, or wait for it to finish with AnimControl::wait_pending() or
 * even PartBundle::wait_pending().  You can also set an event to be triggered
 * when the animation finishes loading with
 * AnimControl::set_pending_done_event().
 */
PT(AnimControl) PartBundle::
load_bind_anim(Loader *loader, const Filename &filename,
               int hierarchy_match_flags, const PartSubset &subset,
               bool allow_async) {
  nassertr(loader != nullptr, nullptr);

  LoaderOptions anim_options(LoaderOptions::LF_search |
                             LoaderOptions::LF_report_errors |
                             LoaderOptions::LF_convert_anim);
  string basename = filename.get_basename_wo_extension();

  int anim_index = -1;
  CPT(AnimPreloadTable) anim_preload = _anim_preload.get_read_pointer();
  if (anim_preload != nullptr) {
    anim_index = anim_preload->find_anim(basename);
  }

  if (anim_index < 0 || !allow_async || !Thread::is_threading_supported()) {
    // The animation is not present in the table, or allow_async is false.
    // Therefore, perform an ordinary synchronous load-and-bind.

    PT(PandaNode) model = loader->load_sync(filename, anim_options);
    if (model == nullptr) {
      // Couldn't load the file.
      return nullptr;
    }
    AnimBundle *anim = AnimBundleNode::find_anim_bundle(model);
    if (anim == nullptr) {
      // No anim bundle.
      return nullptr;
    }
    PT(AnimControl) control = bind_anim(anim, hierarchy_match_flags, subset);
    if (control == nullptr) {
      // Couldn't bind.
      return nullptr;
    }
    control->set_anim_model(model);
    return control;
  }

  // The animation is present in the table, so we can perform an asynchronous
  // load-and-bind.
  PN_stdfloat frame_rate = anim_preload->get_base_frame_rate(anim_index);
  int num_frames = anim_preload->get_num_frames(anim_index);
  PT(AnimControl) control =
    new AnimControl(basename, this, frame_rate, num_frames);

  if (!subset.is_include_empty()) {
    // Figure out the actual subset of joints to be bound.
    int joint_index = 0;
    BitArray bound_joints;
    find_bound_joints(joint_index, false, bound_joints, subset);
    control->set_bound_joints(bound_joints);
  }

  PT(BindAnimRequest) request =
    new BindAnimRequest(string("bind:") + filename.get_basename(),
                        filename, anim_options, loader, control,
                        hierarchy_match_flags, subset);
  request->set_priority(async_bind_priority);
  loader->load_async(request);

  return control;
}

/**
 * Blocks the current thread until all currently-pending AnimControls, with a
 * nonzero control effect, have been loaded and are properly bound.
 */
void PartBundle::
wait_pending() {
  CDReader cdata(_cycler);
  ChannelBlend::const_iterator cbi;
  for (cbi = cdata->_blend.begin();
       cbi != cdata->_blend.end();
       ++cbi) {
    AnimControl *control = (*cbi).first;
    PN_stdfloat effect = (*cbi).second;
    if (effect != 0.0f) {
      control->wait_pending();
    }
  }
}

/**
 * Specifies that the joint with the indicated name should be frozen with the
 * specified transform.  It will henceforth always hold this fixed transform,
 * regardless of any animations that may subsequently be bound to the joint.
 *
 * Returns true if the joint is successfully frozen, or false if the named
 * child is not a joint (or slider) or does not exist.
 */
bool PartBundle::
freeze_joint(const string &joint_name, const TransformState *transform) {
  PartGroup *child = find_child(joint_name);
  if (child == nullptr) {
    return false;
  }

  CDWriter cdata(_cycler, false);
  cdata->_anim_changed = true;

  return child->apply_freeze(transform);
}

/**
 * Specifies that the joint with the indicated name should be frozen with the
 * specified transform.  It will henceforth always hold this fixed transform,
 * regardless of any animations that may subsequently be bound to the joint.
 *
 * Returns true if the joint is successfully frozen, or false if the named
 * child is not a joint (or slider) or does not exist.
 */
bool PartBundle::
freeze_joint(const string &joint_name, const LVecBase3 &pos, const LVecBase3 &hpr, const LVecBase3 &scale) {
  PartGroup *child = find_child(joint_name);
  if (child == nullptr) {
    return false;
  }

  CDWriter cdata(_cycler, false);
  cdata->_anim_changed = true;

  return child->apply_freeze_matrix(pos, hpr, scale);
}

/**
 * Specifies that the joint with the indicated name should be frozen with the
 * specified transform.  It will henceforth always hold this fixed transform,
 * regardless of any animations that may subsequently be bound to the joint.
 *
 * Returns true if the joint is successfully frozen, or false if the named
 * child is not a joint (or slider) or does not exist.
 */
bool PartBundle::
freeze_joint(const string &joint_name, PN_stdfloat value) {
  PartGroup *child = find_child(joint_name);
  if (child == nullptr) {
    return false;
  }

  CDWriter cdata(_cycler, false);
  cdata->_anim_changed = true;

  return child->apply_freeze_scalar(value);
}

/**
 * Specifies that the joint with the indicated name should be animated with
 * the transform on the indicated node.  It will henceforth always follow the
 * node's transform, regardless of any animations that may subsequently be
 * bound to the joint.
 *
 * Returns true if the joint is successfully controlled, or false if the named
 * child is not a joint (or slider) or does not exist.
 */
bool PartBundle::
control_joint(const string &joint_name, PandaNode *node) {
  PartGroup *child = find_child(joint_name);
  if (child == nullptr) {
    return false;
  }

  CDWriter cdata(_cycler, false);
  cdata->_anim_changed = true;

  return child->apply_control(node);
}

/**
 * Releases the named joint from the effects of a previous call to
 * freeze_joint() or control_joint(). It will henceforth once again follow
 * whatever transforms are dictated by the animation.
 *
 * Returns true if the joint is released, or false if the named child was not
 * previously controlled or frozen, or it does not exist.
 */
bool PartBundle::
release_joint(const string &joint_name) {
  PartGroup *child = find_child(joint_name);
  if (child == nullptr) {
    return false;
  }

  CDWriter cdata(_cycler, false);
  cdata->_anim_changed = true;

  return child->clear_forced_channel();
}

/**
 * Updates all the parts in the bundle to reflect the data for the current
 * frame (as set in each of the AnimControls).
 *
 * Returns true if any part has changed as a result of this, or false
 * otherwise.
 */
bool PartBundle::
update() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, false, current_thread);
  bool any_changed = false;

  double now = ClockObject::get_global_clock()->get_frame_time(current_thread);
  if (now > cdata->_last_update + _update_delay || cdata->_anim_changed) {
    bool anim_changed = cdata->_anim_changed;
    bool frame_blend_flag = cdata->_frame_blend_flag;

    any_changed = do_update(this, cdata, nullptr, false, anim_changed,
                            current_thread);

    // Now update all the controls for next time.
    ChannelBlend::const_iterator cbi;
    for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
      AnimControl *control = (*cbi).first;
      control->mark_channels(frame_blend_flag);
    }

    cdata->_anim_changed = false;
    cdata->_last_update = now;
  }

  return any_changed;
}

/**
 * Updates all the parts in the bundle to reflect the data for the current
 * frame, whether we believe it needs it or not.
 */
bool PartBundle::
force_update() {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, false, current_thread);
  bool any_changed = do_update(this, cdata, nullptr, true, true, current_thread);

  // Now update all the controls for next time.
  ChannelBlend::const_iterator cbi;
  for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
    AnimControl *control = (*cbi).first;
    control->mark_channels(cdata->_frame_blend_flag);
  }

  cdata->_anim_changed = false;

  return any_changed;
}


/**
 * Called by the AnimControl whenever it starts an animation.  This is just a
 * hook so the bundle can do something, if necessary, before the animation
 * starts.
 */
void PartBundle::
control_activated(AnimControl *control) {
  nassertv(Thread::get_current_pipeline_stage() == 0);
  nassertv(control->get_part() == this);

  CDLockedReader cdata(_cycler);

  // If (and only if) our anim_blend_flag is false, then starting an animation
  // implicitly enables it.
  if (!cdata->_anim_blend_flag) {
    CDWriter cdataw(_cycler, cdata);
    do_set_control_effect(control, 1.0f, cdataw);
  }
}

/**
 * Called by the AnimControl when it destructs.  This needs to remove the
 * AnimControl pointer from all pipeline stages.
 */
void PartBundle::
control_removed(AnimControl *control) {
  nassertv(control->get_part() == this);

  OPEN_ITERATE_ALL_STAGES(_cycler) {
    CDStageWriter cdata(_cycler, pipeline_stage);
    ChannelBlend::iterator cbi = cdata->_blend.find(control);
    if (cbi != cdata->_blend.end()) {
      cdata->_blend.erase(cbi);
      cdata->_anim_changed = true;

      // We need to make sure that any _effective_channel pointers that point
      // to this control are cleared.
      if (pipeline_stage == 0) {
        determine_effective_channels(cdata);
      }
    }
  }
  CLOSE_ITERATE_ALL_STAGES(_cycler);
}

/**
 * The internal implementation of bind_anim(), this receives a pointer to an
 * uninitialized AnimControl and fills it in if the bind is successful.
 * Returns true if successful, false otherwise.
 */
bool PartBundle::
do_bind_anim(AnimControl *control, AnimBundle *anim,
             int hierarchy_match_flags, const PartSubset &subset) {
  nassertr(Thread::get_current_pipeline_stage() == 0, false);

  // Make sure this pointer doesn't destruct during the lifetime of this
  // method.
  PT(AnimBundle) ptanim = anim;

  if ((hierarchy_match_flags & HMF_ok_wrong_root_name) == 0) {
    // Make sure the root names match.
    if (get_name() != ptanim->get_name()) {
      if (chan_cat.is_error()) {
        chan_cat.error()
          << "Root name of part (" << get_name()
          << ") does not match that of anim (" << ptanim->get_name()
          << ")\n";
      }
      return false;
    }
  }

  if (!check_hierarchy(anim, nullptr, hierarchy_match_flags)) {
    return false;
  }

  plist<int> holes;
  int channel_index = 0;
  pick_channel_index(holes, channel_index);

  if (!holes.empty()) {
    channel_index = holes.front();
  }

  int joint_index = 0;
  BitArray bound_joints;
  if (subset.is_include_empty()) {
    bound_joints = BitArray::all_on();
  }
  bind_hierarchy(ptanim, channel_index, joint_index,
                 subset.is_include_empty(), bound_joints, subset);
  control->setup_anim(this, anim, channel_index, bound_joints);

  CDReader cdata(_cycler);
  determine_effective_channels(cdata);

  return true;
}

/**
 * Adds the PartBundleNode pointer to the set of nodes associated with the
 * PartBundle.  Normally called only by the PartBundleNode itself, for
 * instance when the bundle is flattened with another node.
 */
void PartBundle::
add_node(PartBundleNode *node) {
  nassertv(find(_nodes.begin(), _nodes.end(), node) == _nodes.end());
  _nodes.push_back(node);
}

/**
 * Removes the PartBundleNode pointer from the set of nodes associated with
 * the PartBundle.  Normally called only by the PartBundleNode itself, for
 * instance when the bundle is flattened with another node.
 */
void PartBundle::
remove_node(PartBundleNode *node) {
  Nodes::iterator ni = find(_nodes.begin(), _nodes.end(), node);
  nassertv(ni != _nodes.end());
  _nodes.erase(ni);
}

/**
 * The private implementation of set_control_effect().
 */
void PartBundle::
do_set_control_effect(AnimControl *control, PN_stdfloat effect, CData *cdata) {
  nassertv(control->get_part() == this);

  if (effect == 0.0f) {
    // An effect of zero means to eliminate the control.
    ChannelBlend::iterator cbi = cdata->_blend.find(control);
    if (cbi != cdata->_blend.end()) {
      cdata->_blend.erase(cbi);
      cdata->_anim_changed = true;
    }

  } else {
    // Otherwise we define it.

    // If anim_blend_flag is false, we only allow one AnimControl at a time.
    // Stop all of the other AnimControls.
    if (!cdata->_anim_blend_flag) {
      clear_and_stop_intersecting(control, cdata);
    }

    if (do_get_control_effect(control, cdata) != effect) {
      cdata->_blend[control] = effect;
      cdata->_anim_changed = true;
    }
    cdata->_last_control_set = control;
  }

  determine_effective_channels(cdata);
}

/**
 * The private implementation of get_control_effect().
 */
PN_stdfloat PartBundle::
do_get_control_effect(AnimControl *control, const CData *cdata) const {
  nassertr(control->get_part() == this, 0.0f);

  ChannelBlend::const_iterator cbi = cdata->_blend.find(control);
  if (cbi == cdata->_blend.end()) {
    // The control is not in effect.
    return 0.0f;
  } else {
    return (*cbi).second;
  }
}

/**
 * Removes and stops all the currently activated AnimControls that animate
 * some joints also animated by the indicated AnimControl.  This is a special
 * internal function that's only called when _anim_blend_flag is false, to
 * automatically stop all the other currently-executing animations.
 */
void PartBundle::
clear_and_stop_intersecting(AnimControl *control, CData *cdata) {
  ChannelBlend new_blend;
  bool any_changed = false;

  ChannelBlend::iterator cbi;
  for (cbi = cdata->_blend.begin(); cbi != cdata->_blend.end(); ++cbi) {
    AnimControl *ac = (*cbi).first;
    if (ac == control ||
        !ac->get_bound_joints().has_bits_in_common(control->get_bound_joints())) {
      // Save this control--it's either the target control, or it has no
      // joints in common with the target control.
      new_blend.insert(new_blend.end(), (*cbi));
    } else {
      // Remove and stop this control.
      ac->stop();
      any_changed = true;
    }
  }

  if (any_changed) {
    cdata->_blend.swap(new_blend);
    cdata->_anim_changed = true;
    determine_effective_channels(cdata);
  }
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void PartBundle::
finalize(BamReader *) {
  Thread *current_thread = Thread::get_current_thread();
  CDWriter cdata(_cycler, true);
  do_update(this, cdata, nullptr, true, true, current_thread);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PartBundle::
write_datagram(BamWriter *manager, Datagram &dg) {
  PartGroup::write_datagram(manager, dg);
  manager->write_pointer(dg, _anim_preload.get_read_pointer());
  manager->write_cdata(dg, _cycler);
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int PartBundle::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = PartGroup::complete_pointers(p_list, manager);

  if (manager->get_file_minor_ver() >= 17) {
    _anim_preload = DCAST(AnimPreloadTable, p_list[pi++]);
  }

  return pi;
}

/**
 * Factory method to generate a PartBundle object
 */
TypedWritable* PartBundle::
make_from_bam(const FactoryParams &params) {
  PartBundle *me = new PartBundle;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  manager->register_finalize(me);
  return me;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PartBundle.
 */
void PartBundle::
fillin(DatagramIterator &scan, BamReader *manager) {
  PartGroup::fillin(scan, manager);
  if (manager->get_file_minor_ver() >= 17) {
    manager->read_pointer(scan);  // _anim_preload
  }
  if (manager->get_file_minor_ver() >= 10) {
    manager->read_cdata(scan, _cycler);
  }
  if (manager->get_file_minor_ver() == 11) {
    // No longer need the _modifies_anim_bundles flag
    scan.get_bool();
  }
}

/**
 * Factory method to generate a PartBundle object
 */
void PartBundle::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 *
 */
PartBundle::CData::
CData() {
  _blend_type = anim_blend_type;
  _anim_blend_flag = false;
  _frame_blend_flag = interpolate_frames;
  _root_xform = LMatrix4::ident_mat();
  _last_control_set = nullptr;
  _anim_changed = false;
  _last_update = 0.0;
}

/**
 *
 */
PartBundle::CData::
CData(const PartBundle::CData &copy) :
  _blend_type(copy._blend_type),
  _anim_blend_flag(copy._anim_blend_flag),
  _frame_blend_flag(copy._frame_blend_flag),
  _root_xform(copy._root_xform),
  _last_control_set(copy._last_control_set),
  _blend(copy._blend),
  _anim_changed(copy._anim_changed),
  _last_update(copy._last_update)
{
  // Note that this copy constructor is not used by the PartBundle copy
  // constructor!  Any elements that must be copied between PartBundles should
  // also be explicitly copied there.
}

/**
 *
 */
CycleData *PartBundle::CData::
make_copy() const {
  return new CData(*this);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void PartBundle::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  dg.add_uint8(_blend_type);
  dg.add_bool(_anim_blend_flag);
  dg.add_bool(_frame_blend_flag);
  _root_xform.write_datagram(dg);

  // The remaining members are strictly dynamic.
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new PartBundle.
 */
void PartBundle::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _blend_type = (BlendType)scan.get_uint8();
  _anim_blend_flag = scan.get_bool();
  _frame_blend_flag = scan.get_bool();
  _root_xform.read_datagram(scan);
}

/**
 *
 */
ostream &
operator << (ostream &out, PartBundle::BlendType blend_type) {
  switch (blend_type) {
    case PartBundle::BT_linear:
      return out << "linear";

    case PartBundle::BT_normalized_linear:
      return out << "normalized_linear";

    case PartBundle::BT_componentwise:
      return out << "componentwise";

    case PartBundle::BT_componentwise_quat:
      return out << "componentwise_quat";
  }

  chan_cat->error()
    << "Invalid BlendType value: " << (int)blend_type << "\n";
  nassertr(false, out);
  return out;
}

/**
 *
 */
istream &
operator >> (istream &in, PartBundle::BlendType &blend_type) {
  string word;
  in >> word;

  if (cmp_nocase_uh(word, "linear") == 0) {
    blend_type = PartBundle::BT_linear;

  } else if (cmp_nocase_uh(word, "normalized_linear") == 0) {
    blend_type = PartBundle::BT_normalized_linear;

  } else if (cmp_nocase_uh(word, "componentwise") == 0) {
    blend_type = PartBundle::BT_componentwise;

  } else if (cmp_nocase_uh(word, "componentwise_quat") == 0) {
    blend_type = PartBundle::BT_componentwise_quat;

  } else {
    chan_cat->error()
      << "Invalid BlendType string: " << word << "\n";
    blend_type = PartBundle::BT_linear;
  }
  return in;
}
