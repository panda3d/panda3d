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
//     Function: FadeLODNode::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *FadeLODNode::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void FadeLODNode::CData::
write_datagram(BamWriter *manager, Datagram &dg) const {
  _lod.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LODNode.
////////////////////////////////////////////////////////////////////
void FadeLODNode::CData::
fillin(DatagramIterator &scan, BamReader *manager) {
  _lod.read_datagram(scan);
}

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
//     Function: FadeLODNode::safe_to_combine
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to combine this
//               particular kind of PandaNode with other kinds of
//               PandaNodes, adding children or whatever.  For
//               instance, an LODNode should not be combined with any
//               other PandaNode, because its set of children is
//               meaningful.
////////////////////////////////////////////////////////////////////
bool FadeLODNode::
safe_to_combine() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void FadeLODNode::
xform(const LMatrix4f &mat) {
  CDWriter cdata(_cycler);
  cdata->_lod.xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: FadeLODNode::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool FadeLODNode::
has_cull_callback() const {
  return true;
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
  PandaNode *node = data.node();
  CDReader cdata(_cycler);
  if (_fade_mode) {
    float in_alpha;
    float out_alpha;
    _fade_timer -= ClockObject::get_global_clock()->get_dt();
      if (_fade_timer <= (cdata->_fade_time / 2.0)) { 
        //SECOND HALF OF FADE:
        //Fade out the old LOD with z write off and 
        //draw the opaque new LOD with z write on
        out_alpha = (_fade_timer*2.0) / cdata->_fade_time;  
        if (out_alpha < 0.0) {
          out_alpha = 0.0;
        }
        
        CullTraverserData next_data_in(data, node->get_child(_fade_in));
        CullTraverserData next_data_out(data, node->get_child(_fade_out));
        
        // Disable Transparency on new LOD
        next_data_in._state = next_data_in._state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_none), 0);
        // Enable Transparency on old LOD
        next_data_out._state = next_data_out._state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha), 0);
        // Start Fading the old LOD out
        next_data_out._state = next_data_out._state->add_attrib(ColorScaleAttrib::make(LVecBase4f(1.0,1.0,1.0,out_alpha)));
        // The new LOD is now opaque and has depth writing
        next_data_in._state = next_data_in._state->add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_on), 0);
        // The old LOD is fading so it doesnt depth write
        next_data_out._state = next_data_out._state->add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off), 0);
        
        
        trav->traverse(next_data_in);
        trav->traverse(next_data_out);
      } else {
        // FIRST HALF OF FADE
        // Fade the new LOD in with z writing off
        // Keep drawing the old LOD opaque with z writing on
        in_alpha = (1.0 - (_fade_timer / cdata->_fade_time))*2.0;  
         if (in_alpha > 1.0) {
          in_alpha = 1.0;
        }
        
        CullTraverserData next_data_out(data, node->get_child(_fade_out));
        CullTraverserData next_data_in(data, node->get_child(_fade_in));

        // Disable transparency on old LOD
        next_data_out._state = next_data_out._state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_none), 0);
        // Enable transparency on new LOD
        next_data_in._state = next_data_in._state->add_attrib(TransparencyAttrib::make(TransparencyAttrib::M_alpha), 0);
        // Start Fading in the new LOD
        next_data_in._state = next_data_in._state->add_attrib(ColorScaleAttrib::make(LVecBase4f(1.0,1.0,1.0,in_alpha)));
        // Enable depth write for the old LOD
        next_data_out._state = next_data_out._state->add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_on), 0);
        // Disable depth write for the new LOD
        next_data_in._state = next_data_in._state->add_attrib(DepthWriteAttrib::make(DepthWriteAttrib::M_off), 0);
        
        trav->traverse(next_data_out);
        trav->traverse(next_data_in);
      }
    if (_fade_timer < 0) { // Fading Complete
      _fade_mode = false;
    }
  } else {
    if (data._net_transform->is_singular()) {
      // If we're under a singular transform, we can't compute the LOD;
      // select none of them instead.
      //select_child(get_num_children());
      return false;
    } else { 
      LPoint3f camera_pos(0, 0, 0);
      // Get the LOD center in camera space
      CPT(TransformState) rel_transform =
        trav->get_camera_transform()->invert_compose(data._net_transform);
      LPoint3f center = cdata->_lod._center * rel_transform->get_mat();
      // Determine which child to traverse 
      int index = cdata->_lod.compute_child(camera_pos, center);
      //printf("CHILD: %d PREVIOUS %d \n",index,_previous_child);
      if (index != _previous_child) { // Transition occurred
        _fade_mode = true;
        _fade_timer = cdata->_fade_time;
        _fade_out = _previous_child; 
        _fade_in = index;
        _previous_child = index;
        CullTraverserData next_data_transition(data, node->get_child(_fade_out));
        trav->traverse(next_data_transition);
      } else {
        // No transition... handle things as usual
        // Traverse only one valid child
        CullTraverserData next_data_normal(data, node->get_child(index));
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
 PandaNode::output(out);
  CDReader cdata(_cycler);
  out << " ";
  cdata->_lod.output(out);
  out<< "Fade Time : " << cdata->_fade_time << endl;
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
  PandaNode::write_datagram(manager, dg);
  manager->write_cdata(dg, _cycler);
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
  PandaNode::fillin(scan, manager);
  manager->read_cdata(scan, _cycler);
}
