// Filename: sequenceNode.h
// Created by:  jason (18Jul00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>
#include <clockObject.h>

#include "sequenceNode.h"
#include "config_switchnode.h"

#include <graphicsStateGuardian.h>
#include <allAttributesWrapper.h>
#include <allTransitionsWrapper.h>
#include <renderRelation.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle SequenceNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SequenceNode::
SequenceNode(const string &initial_name) :
  NamedNode(initial_name), TimedCycle()
{
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SequenceNode::
SequenceNode(float switch_time, const string &initial_name) :
   NamedNode(initial_name), TimedCycle(switch_time, 0)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::set_switch_time
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void SequenceNode::
set_switch_time(float switch_time)
{
  set_cycle_time(switch_time);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::sub_render
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool SequenceNode::
sub_render(const AllAttributesWrapper &attrib, AllTransitionsWrapper &trans,
	   GraphicsStateGuardianBase *gsgbase) 
{
  GraphicsStateGuardian *gsg = DCAST(GraphicsStateGuardian, gsgbase);

  // Determine which child to traverse
  int num_children = get_num_children(RenderRelation::get_class_type());
  set_element_count(num_children);
  int index = next_element();
  if (index >= 0 && index < num_children) {
    NodeRelation *arc = get_child(RenderRelation::get_class_type(), index);

    if (switchnode_cat.is_debug()) {
      switchnode_cat.debug()
	<< "Selecting child " << index << " of " << *this << ": "
	<< *arc->get_child() << "\n";
    }

    // We have to be sure to pick up any state transitions on the arc
    // itself.
    AllTransitionsWrapper arc_trans;
    arc_trans.extract_from(arc);

    AllTransitionsWrapper new_trans(trans);
    new_trans.compose_in_place(arc_trans);

    // Now render everything from this node and below.
    gsg->render_subgraph(gsg->get_render_traverser(), 
			 arc->get_child(), attrib, new_trans);

  } else {
    if (switchnode_cat.is_debug()) {
      switchnode_cat.debug()
	<< "Cannot select child " << index << " of " << *this << "; only "
	<< num_children << " children.\n";
    }
  }

  // Short circuit the rest of the render pass below this node
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::has_sub_render
//       Access: Public, Virtual
//  Description: Should be redefined to return true if the function
//               sub_render(), above, expects to be called during
//               traversal.
////////////////////////////////////////////////////////////////////
bool SequenceNode::
has_sub_render() const 
{
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::write_object
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void SequenceNode::
write_datagram(BamWriter *manager, Datagram &me) {
  NamedNode::write_datagram(manager, me);
  TimedCycle::write_datagram(me);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_SequenceNode to
//               read in all of the relevant data from the BamFile for
//               the new SequenceNode.
////////////////////////////////////////////////////////////////////
void SequenceNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  NamedNode::fillin(scan, manager);
  TimedCycle::fillin(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::make_SequenceNode
//       Access: Protected
//  Description: This function is called by the BamReader's factory
//               when a new object of type SequenceNode is encountered in
//               the Bam file.  It should create the SequenceNode and
//               extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWriteable *SequenceNode::
make_SequenceNode(const FactoryParams &params) {
  SequenceNode *me = new SequenceNode;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: SequenceNode::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               SequenceNode.
////////////////////////////////////////////////////////////////////
void SequenceNode::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_SequenceNode);
}
