// Filename: fadeLodNode.cxx
// Created by:  sshodhan (14Jun04)
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

#include "fadeLodNode.h"
#include "cullTraverserData.h"
#include "cullTraverser.h"
#include "clockObject.h"
#include "colorScaleAttrib.h"
#include "depthWriteAttrib.h"
#include "transparencyAttrib.h"

TypeHandle FadeLODNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *FadeLODNode::
make_copy() const {
  return new FadeLODNode(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
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
bool FadeLODNode::
cull_callback(CullTraverser *trav, CullTraverserData &data) {
  if (_fade_mode) {
    float now = ClockObject::get_global_clock()->get_frame_time();
    float elapsed = now - _fade_start;

    float half_fade_time = _fade_time / 2.0f;

    if (elapsed < half_fade_time) { 
      // FIRST HALF OF FADE
      // Fade the new LOD in with z writing off
      // Keep drawing the old LOD opaque with z writing on
      if (_fade_out >= 0 && _fade_out < get_num_children()) {
        CullTraverserData next_data_out(data, get_child(_fade_out));
        trav->traverse(next_data_out);
      }

      if (_fade_in >= 0 && _fade_in < get_num_children()) {
        CullTraverserData next_data_in(data, get_child(_fade_in));

        float in_alpha = elapsed / half_fade_time;
        if (in_alpha > 1.0f) {
          in_alpha = 1.0f;
        }
        LVecBase4f alpha_scale(1.0f, 1.0f, 1.0f, in_alpha);

        next_data_in._state = 
          next_data_in._state->compose(get_fade_out_state())->compose
          (RenderState::make(ColorScaleAttrib::make(alpha_scale)));

        trav->traverse(next_data_in);
      }

    } else if (elapsed < _fade_time) {
      //SECOND HALF OF FADE:
      //Fade out the old LOD with z write off and 
      //draw the opaque new LOD with z write on
      if (_fade_in >= 0 && _fade_in < get_num_children()) {
        CullTraverserData next_data_in(data, get_child(_fade_in));
        trav->traverse(next_data_in);
      }

      if (_fade_out >= 0 && _fade_out < get_num_children()) {
        CullTraverserData next_data_out(data, get_child(_fade_out));
        
        float out_alpha = 1.0f - elapsed / half_fade_time;  
        if (out_alpha < 0.0f) {
          out_alpha = 0.0f;
        }
        LVecBase4f alpha_scale(1.0f, 1.0f, 1.0f, out_alpha);

        next_data_out._state = 
          next_data_out._state->compose(get_fade_out_state())->compose
          (RenderState::make(ColorScaleAttrib::make(alpha_scale)));

        trav->traverse(next_data_out);
      }

    } else {
      // Fading complete
      _fade_mode = false;
      
      if (_fade_in >= 0 && _fade_in < get_num_children()) {
        CullTraverserData next_data_in(data, get_child(_fade_in));
        trav->traverse(next_data_in);
      }
    }

  } else {
    int index = compute_child(trav, data);
    if (index != _previous_child) { // Transition occurred
      _fade_mode = true;
      _fade_start = ClockObject::get_global_clock()->get_frame_time();
      _fade_out = _previous_child; 
      _fade_in = index;
      _previous_child = index;
      if (_fade_out >= 0 && _fade_out < get_num_children()) {
        CullTraverserData next_data_transition(data, get_child(_fade_out));
        trav->traverse(next_data_transition);
      }
      
    } else {
      if (index >= 0 && index < get_num_children()) {
        // No transition... handle things as usual
        // Traverse only one valid child
        CullTraverserData next_data_normal(data, get_child(index));
        trav->traverse(next_data_normal);
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void FadeLODNode::
output(ostream &out) const {
  LODNode::output(out);
  out << " fade time: " << _fade_time;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::get_fade_out_state
//       Access: Protected, Static
//  Description: Returns a RenderState for rendering the element that
//               is switching out of visibility.
////////////////////////////////////////////////////////////////////
CPT(RenderState) FadeLODNode::
get_fade_out_state() {
  // Once someone asks for this pointer, we hold its reference count
  // and never free it.
  static CPT(RenderState) state = (const RenderState *)NULL;
  if (state == (const RenderState *)NULL) {
    state = RenderState::make
      (TransparencyAttrib::make(TransparencyAttrib::M_alpha),
       DepthWriteAttrib::make(DepthWriteAttrib::M_off));
  }

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LODNode.
////////////////////////////////////////////////////////////////////
void FadeLODNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void FadeLODNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  LODNode::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type LODNode is encountered
//               in the Bam file.  It should create the LODNode
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *FadeLODNode::
make_from_bam(const FactoryParams &params) {
  FadeLODNode *node = new FadeLODNode("");
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  node->fillin(scan, manager);

  return node;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new FadeLODNode.
////////////////////////////////////////////////////////////////////
void FadeLODNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  LODNode::fillin(scan, manager);
}
