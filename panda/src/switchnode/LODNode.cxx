// Filename: LODNode.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "LODNode.h"
#include "projectionNode.h"
#include "config_switchnode.h"

#include <graphicsStateGuardian.h>
#include <get_rel_pos.h>
#include <luse.h>
#include <renderRelation.h>
#include <transformTransition.h>
#include <allAttributesWrapper.h>
#include <allTransitionsWrapper.h>
#include <renderTraverser.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle LODNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LODNode::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.  No children will be
//               copied.
////////////////////////////////////////////////////////////////////
Node *LODNode::
make_copy() const {
  return new LODNode(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: LODNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this node by the indicated
//               matrix, if it means anything to do so.
////////////////////////////////////////////////////////////////////
void LODNode::
xform(const LMatrix4f &mat) {
  _lod.xform(mat);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LODNode::
output(ostream &out) const {
  SwitchNodeOne::output(out);
  out << " ";
  _lod.output(out);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::compute_switch
//       Access: Public, Virtual
//  Description: Computes the node's switching properties, to decide
//               what children should be visible in the given
//               rendering situation.
//
//               In the case of an LODNode, this decides which child
//               is visible based on the distance from the camera.
////////////////////////////////////////////////////////////////////
void LODNode::
compute_switch(RenderTraverser *trav) {
  GraphicsStateGuardian *gsg = trav->get_gsg();

  // Get the current camera position from the gsg
  const ProjectionNode* camera = gsg->get_current_projection_node();
  LPoint3f camera_pos(0, 0, 0);

  // Get the LOD center in camera space
  LPoint3f LOD_pos;

  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(this, trav->begin(), trav->end(),
      camera, ntw, RenderRelation::get_class_type());
  const TransformTransition *tt;
  if (get_transition_into(tt, ntw)) {
    LOD_pos = _lod._center * tt->get_matrix();
  } else {
    LOD_pos = _lod._center;
  }
  
  // Determine which child to traverse
  int index = _lod.compute_child(camera_pos, LOD_pos);
  select_child(index);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LODNode.
////////////////////////////////////////////////////////////////////
void LODNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_LODNode);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::write_object
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LODNode::
write_datagram(BamWriter *manager, Datagram &me) {
  SwitchNodeOne::write_datagram(manager, me);
  _lod.write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::make_LODNode
//       Access: Protected
//  Description: This function is called by the BamReader's factory
//               when a new object of type LODNode is encountered in
//               the Bam file.  It should create the LODNode and
//               extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *LODNode::
make_LODNode(const FactoryParams &params) {
  LODNode *me = new LODNode;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_LODNode to
//               read in all of the relevant data from the BamFile for
//               the new LODNode.
////////////////////////////////////////////////////////////////////
void LODNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  SwitchNodeOne::fillin(scan, manager);
  _lod.read_datagram(scan);
}
