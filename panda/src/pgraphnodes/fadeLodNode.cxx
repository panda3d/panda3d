/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fadeLodNode.cxx
 * @author sshodhan
 * @date 2004-06-14
 */

#include "fadeLodNode.h"
#include "fadeLodNodeData.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "clockObject.h"
#include "colorScaleAttrib.h"
#include "depthWriteAttrib.h"
#include "transparencyAttrib.h"
#include "cullBinAttrib.h"
#include "depthOffsetAttrib.h"

TypeHandle FadeLODNode::_type_handle;

/**
 *
 */
FadeLODNode::
FadeLODNode(const std::string &name) :
  LODNode(name)
{
  set_cull_callback();

  _fade_time = lod_fade_time;
  _fade_bin_name = lod_fade_bin_name;
  _fade_bin_draw_order = lod_fade_bin_draw_order;
  _fade_state_override = lod_fade_state_override;
}

/**
 *
 */
FadeLODNode::
FadeLODNode(const FadeLODNode &copy) :
  LODNode(copy)
{
  _fade_time = copy._fade_time;
  _fade_bin_name = copy._fade_bin_name;
  _fade_bin_draw_order = copy._fade_bin_draw_order;
  _fade_state_override = copy._fade_state_override;
}

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *FadeLODNode::
make_copy() const {
  return new FadeLODNode(*this);
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
bool FadeLODNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  if (!support_fade_lod) {
    return LODNode::cull_callback(trav, data);
  }

  if (is_any_shown()) {
    return show_switches_cull_callback(trav, data);
  }

  consider_verify_lods(trav, data);

  Camera *camera = trav->get_scene()->get_camera_node();
  NodePath this_np = data.get_node_path();
  FadeLODNodeData *ldata =
    DCAST(FadeLODNodeData, camera->get_aux_scene_data(this_np));

  double now = ClockObject::get_global_clock()->get_frame_time();

  if (ldata == nullptr || now > ldata->get_expiration_time()) {
    // This is the first time we have rendered this instance of this LOD node
    // in a while.
    ldata = new FadeLODNodeData;
    ldata->_fade_mode = FadeLODNodeData::FM_solid;
    ldata->_fade_out = -1;
    ldata->_fade_in = compute_child(trav, data);
    camera->set_aux_scene_data(this_np, ldata);

  } else {
    // We had rendered this LOD node last frame (or not too long ago, at
    // least).

    if (ldata->_fade_mode == FadeLODNodeData::FM_solid) {
      // We were drawing just one solid child last frame; check whether it's
      // time to begin a transition.
      int index = compute_child(trav, data);
      if (index != ldata->_fade_in) {
        // Start a transition.
        if (index >= 0 && ldata->_fade_in >= 0 &&
            get_out(index) > get_out(ldata->_fade_in)) {
          // We are fading from a more-detailed model to a less-detailed
          // model.
          ldata->_fade_mode = FadeLODNodeData::FM_less_detail;
        } else {
          // We are fading from a less-detailed model to a more-detailed
          // model.
          ldata->_fade_mode = FadeLODNodeData::FM_more_detail;
        }

        // We start the fade as of the last frame we actually rendered; that
        // way, if the object happened to be offscreen for a large part of the
        // fade, we'll just view the tail end of it--a little nicer.
        ldata->_fade_start = ldata->get_last_render_time();
        ldata->_fade_out = ldata->_fade_in;
        ldata->_fade_in = index;
      }
    }

    if (ldata->_fade_mode != FadeLODNodeData::FM_solid) {
      // Play the transition.

      PN_stdfloat elapsed = now - ldata->_fade_start;

      if (elapsed >= _fade_time) {
        // Transition complete.
        ldata->_fade_mode = FadeLODNodeData::FM_solid;

      } else {
        PN_stdfloat half_fade_time = _fade_time * 0.5f;

        int in_child = ldata->_fade_in;
        int out_child = ldata->_fade_out;

        if (ldata->_fade_mode == FadeLODNodeData::FM_less_detail) {
          // If we're fading from a more-detailed model to a less-detailed
          // model, reverse the fade effect for best visual quality.
          elapsed = _fade_time - elapsed;
          int t = in_child;
          in_child = out_child;
          out_child = t;
        }

        nassertr(elapsed >= 0.0f && elapsed <= _fade_time, false);

        if (elapsed < half_fade_time) {
          // FIRST HALF OF FADE Fade the new LOD in with z writing off Keep
          // drawing the old LOD opaque with z writing on
          if (out_child >= 0 && out_child < get_num_children()) {
            PandaNode *child = get_child(out_child);
            if (child != nullptr) {
              CullTraverserData next_data_out(data, child);
              next_data_out._state =
                next_data_out._state->compose(get_fade_1_old_state());
              trav->traverse(next_data_out);
            }
          }

          if (in_child >= 0 && in_child < get_num_children()) {
            PandaNode *child = get_child(in_child);
            if (child != nullptr) {
              CullTraverserData next_data_in(data, child);

              PN_stdfloat in_alpha = elapsed / half_fade_time;
              next_data_in._state =
                next_data_in._state->compose(get_fade_1_new_state(in_alpha));
              trav->traverse(next_data_in);
            }
          }

        } else {
          // SECOND HALF OF FADE: Fade out the old LOD with z write off and
          // draw the opaque new LOD with z write on
          if (in_child >= 0 && in_child < get_num_children()) {
            PandaNode *child = get_child(in_child);
            if (child != nullptr) {
              CullTraverserData next_data_in(data, child);
              next_data_in._state =
                next_data_in._state->compose(get_fade_2_new_state());
              trav->traverse(next_data_in);
            }
          }

          if (out_child >= 0 && out_child < get_num_children()) {
            PandaNode *child = get_child(out_child);
            if (child != nullptr) {
              CullTraverserData next_data_out(data, child);

              PN_stdfloat out_alpha = 1.0f - (elapsed - half_fade_time) / half_fade_time;
              next_data_out._state =
                next_data_out._state->compose(get_fade_2_old_state(out_alpha));
              trav->traverse(next_data_out);
            }
          }
        }
      }
    }
  }

  if (ldata->_fade_mode == FadeLODNodeData::FM_solid) {
    // This is the normal case: we're not in the middle of a transition; we're
    // just drawing one child of the LOD.
    int index = ldata->_fade_in;
    if (index >= 0 && index < get_num_children()) {
      PandaNode *child = get_child(index);
      if (child != nullptr) {
        CullTraverserData next_data(data, child);
        trav->traverse(next_data);
      }
    }
  }

  ldata->set_last_render_time(now);
  ldata->set_duration(_fade_time);

  return false;
}

/**
 *
 */
void FadeLODNode::
output(std::ostream &out) const {
  LODNode::output(out);
  out << " fade time: " << _fade_time;
}

/**
 * Specifies the cull bin and draw order that is assigned to the fading part
 * of the geometry during a transition.
 */
void FadeLODNode::
set_fade_bin(const std::string &name, int draw_order) {
  _fade_bin_name = name;
  _fade_bin_draw_order = draw_order;
  _fade_1_new_state.clear();
  _fade_2_old_state.clear();
}

/**
 * Specifies the override value that is applied to the state changes necessary
 * to apply the fade effect.  This should be larger than any attrib overrides
 * on the fading geometry.
 */
void FadeLODNode::
set_fade_state_override(int override) {
  _fade_state_override = override;
  _fade_1_old_state.clear();
  _fade_1_new_state.clear();
  _fade_2_old_state.clear();
  _fade_2_new_state.clear();
}

/**
 * Returns a RenderState for rendering the old element during first half of
 * fade.
 */
CPT(RenderState) FadeLODNode::
get_fade_1_old_state() {
  if (_fade_1_old_state == nullptr) {
    _fade_1_old_state = RenderState::make_empty();
  }

  return _fade_1_old_state;
}

/**
 * Returns a RenderState for rendering the new element during first half of
 * fade.
 */
CPT(RenderState) FadeLODNode::
get_fade_1_new_state(PN_stdfloat in_alpha) {
  if (_fade_1_new_state == nullptr) {
    _fade_1_new_state = RenderState::make
      (TransparencyAttrib::make(TransparencyAttrib::M_alpha),
       CullBinAttrib::make(_fade_bin_name, _fade_bin_draw_order),
       DepthOffsetAttrib::make(),
       _fade_state_override);
  }

  LVecBase4 alpha_scale(1.0f, 1.0f, 1.0f, in_alpha);
  return _fade_1_new_state->compose
    (RenderState::make(ColorScaleAttrib::make(alpha_scale)));
}

/**
 * Returns a RenderState for rendering the old element during second half of
 * fade.
 */
CPT(RenderState) FadeLODNode::
get_fade_2_old_state(PN_stdfloat out_alpha) {
  if (_fade_2_old_state == nullptr) {
    _fade_2_old_state = RenderState::make
      (TransparencyAttrib::make(TransparencyAttrib::M_alpha),
       DepthWriteAttrib::make(DepthWriteAttrib::M_off),
       CullBinAttrib::make(_fade_bin_name, _fade_bin_draw_order),
       _fade_state_override);
  }

  LVecBase4 alpha_scale(1.0f, 1.0f, 1.0f, out_alpha);
  return _fade_2_old_state->compose
    (RenderState::make(ColorScaleAttrib::make(alpha_scale)));
}

/**
 * Returns a RenderState for rendering the new element during second half of
 * fade.
 */
CPT(RenderState) FadeLODNode::
get_fade_2_new_state() {
  if (_fade_2_new_state == nullptr) {
    _fade_2_new_state = RenderState::make
      (DepthOffsetAttrib::make(),
       _fade_state_override);
  }

  return _fade_2_new_state;
}

/**
 * Tells the BamReader how to create objects of type LODNode.
 */
void FadeLODNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void FadeLODNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  LODNode::write_datagram(manager, dg);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type LODNode is encountered in the Bam file.  It should create the LODNode
 * and extract its information from the file.
 */
TypedWritable *FadeLODNode::
make_from_bam(const FactoryParams &params) {
  FadeLODNode *node = new FadeLODNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new FadeLODNode.
 */
void FadeLODNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  LODNode::fillin(scan, manager);
}
