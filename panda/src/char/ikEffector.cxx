/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ikEffector.cxx
 * @author rdb
 * @date 2020-11-16
 */

#include "ikEffector.h"
#include "pandaNode.h"

TypeHandle IKEffector::_type_handle;

/**
 * Creates a new IKEffector tracking the given node.
 */
IKEffector::
IKEffector(PartGroup *parent, PandaNode *node) :
  PartGroup(parent, ""),
  _node(node) {
}

/**
 * Recursively initializes the joint for IK, calculating the current net
 * position and lengths.  Returns true if there were any effectors under this
 * node, false otherwise.
 */
bool IKEffector::
r_init_ik(const LPoint3 &parent_pos) {
  _ik_pos = _node->get_transform()->get_pos();
  _length = (_ik_pos - parent_pos).length();
  return true;
}

/**
 * Executes a forward IK pass on the given points (which are set up by
 * r_setup_ik_points).
 */
void IKEffector::
r_forward_ik(const LPoint3 &parent_pos) {
  // Nothing to do here.  End effectors only take effect in the reverse pass.
}

/**
 * Executes a reverse IK pass on the given points (which are set up by
 * r_setup_ik_points).  Returns true if there were any effectors under this
 * joint, in which case the new position of this joint is stored in out_pos.
 */
bool IKEffector::
r_reverse_ik(LPoint3 &out_pos) {
  out_pos = _ik_pos;
  return true;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void IKEffector::
write_datagram(BamWriter *manager, Datagram &me) {
  PartGroup::write_datagram(manager, me);
}

/**
 * Takes in a vector of pointers to TypedWritable objects that correspond to
 * all the requests for pointers that this object made to BamReader.
 */
int IKEffector::
complete_pointers(TypedWritable **p_list, BamReader* manager) {
  int pi = PartGroup::complete_pointers(p_list, manager);

  _node = DCAST(PandaNode, p_list[pi++]);

  return pi;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void IKEffector::
fillin(DatagramIterator &scan, BamReader *manager) {
  PartGroup::fillin(scan, manager);
}

/**
 * Factory method to generate a IKEffector object
 */
/*TypedWritable* IKEffector::
make_IKEffector(const FactoryParams &params) {
  IKEffector *me = new IKEffector;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}*/

/**
 * Factory method to generate a IKEffector object
 */
/*void IKEffector::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_IKEffector);
}*/
