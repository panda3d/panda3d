// Filename: billboardTransition.cxx
// Created by:  mike (06Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "billboardTransition.h"
#include "transformTransition.h"
#include "transformAttribute.h"
#include "renderRelation.h"

#include <graphicsStateGuardian.h>
#include <renderTraverser.h>
#include <projectionNode.h>
#include <look_at.h>
#include <nodeTransitionWrapper.h>
#include <wrt.h>
#include <indent.h>
#include <allTransitionsWrapper.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle BillboardTransition::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
NodeTransition *BillboardTransition::
make_copy() const {
  return new BillboardTransition(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::sub_render 
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool BillboardTransition::
sub_render(NodeRelation *arc, const AllAttributesWrapper &,
	   AllTransitionsWrapper &trans, RenderTraverser *trav) {
  Node *node = arc->get_child();
  GraphicsStateGuardian *gsg = trav->get_gsg();

  // Get the current camera from the gsg
  const ProjectionNode *camera = gsg->get_current_projection_node();
  nassertr(camera != (ProjectionNode *)NULL, true);

  // And the relative coordinate space.
  LMatrix4f rel_mat;
  NodeTransitionWrapper ntw(TransformTransition::get_class_type());
  wrt(camera, node, trav->begin(), trav->end(), 
      ntw, RenderRelation::get_class_type());
  TransformTransition *tt;
  if (!get_transition_into(tt, ntw)) {
    // No relative transform.
    rel_mat = LMatrix4f::ident_mat();
  } else {
    rel_mat = tt->get_matrix();
  }

  LVector3f camera_pos = -rel_mat.get_row3(3);
  LVector3f up = _up_vector;

  // If this is an eye-relative Billboard, then (a) the up vector is
  // relative to the camera, not to the world, and (b) the look
  // direction is towards the plane that contains the camera,
  // perpendicular to the forward direction, not directly to the
  // camera.
  if (_eye_relative) {
    up = _up_vector * rel_mat;
    camera_pos = LVector3f::forward(gsg->get_coordinate_system()) * rel_mat;
  }

  // Now determine the rotation matrix for the Billboard.
  LMatrix4f rotate;
  if (_axial_rotate) {
    heads_up(rotate, camera_pos, up, gsg->get_coordinate_system());
  } else {
    look_at(rotate, camera_pos, up, gsg->get_coordinate_system());
  }

  // And finally, apply the rotation transform to the set of
  // transitions we've accumulated for this node.
  AllTransitionsWrapper new_trans;
  new_trans.set_transition(new TransformTransition(rotate));

  trans.compose_in_place(new_trans);

  // Continue the render pass
  return true; 
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool BillboardTransition::
has_sub_render() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BillboardTransition::
output(ostream &out) const {
  if (_axial_rotate) {
    out << "axial";
  } else {
    out << "point";
  }
  if (_eye_relative) {
    out << " eye";
  }
  out << " Billboard";
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void BillboardTransition::
write(ostream &out, int indent_level) const {
  indent(out, indent_level);
  if (_axial_rotate) {
    out << "axial";
  } else {
    out << "point";
  }
  if (_eye_relative) {
    out << " eye";
  }
  out << " Billboard:\n";

  indent(out, indent_level + 2) 
    << "up vector: " << _up_vector << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::internal_compare_to
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
int BillboardTransition::
internal_compare_to(const NodeTransition *other) const {
  const BillboardTransition *ot;
  DCAST_INTO_R(ot, other, false);

  if (_eye_relative != ot->_eye_relative) {
    return (int)_eye_relative - (int)ot->_eye_relative;
  }
  if (_axial_rotate != ot->_axial_rotate) {
    return (int)_axial_rotate - (int)ot->_axial_rotate;
  }
  return _up_vector.compare_to(ot->_up_vector);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void BillboardTransition::
write_datagram(BamWriter *manager, Datagram &me)
{
  ImmediateTransition::write_datagram(manager, me);
  _up_vector.write_datagram(me);
  me.add_uint8(_eye_relative);
  me.add_uint8(_axial_rotate);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void BillboardTransition::
fillin(DatagramIterator& scan, BamReader* manager)
{
  ImmediateTransition::fillin(scan, manager);
  _up_vector.read_datagram(scan);
  _eye_relative = (scan.get_uint8() !=0);
  _axial_rotate = (scan.get_uint8() !=0);
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::make_BillboardTransition
//       Access: Protected
//  Description: Factory method to generate a BillboardTransition object
////////////////////////////////////////////////////////////////////
TypedWritable* BillboardTransition::
make_BillboardTransition(const FactoryParams &params)
{
  BillboardTransition *me = new BillboardTransition;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: BillboardTransition::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a BillboardTransition object
////////////////////////////////////////////////////////////////////
void BillboardTransition::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_BillboardTransition);
}

