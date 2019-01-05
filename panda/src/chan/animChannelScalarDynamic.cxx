/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelScalarDynamic.cxx
 * @author drose
 * @date 2003-10-20
 */

#include "animChannelScalarDynamic.h"
#include "animBundle.h"
#include "config_chan.h"
#include "transformState.h"
#include "pandaNode.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle AnimChannelScalarDynamic::_type_handle;

/**
 * For use only with the bam reader.
 */
AnimChannelScalarDynamic::
AnimChannelScalarDynamic() {
}

/**
 * Creates a new AnimChannelScalarDynamic, just like this one, without copying
 * any children.  The new copy is added to the indicated parent.  Intended to
 * be called by make_copy() only.
 */
AnimChannelScalarDynamic::
AnimChannelScalarDynamic(AnimGroup *parent, const AnimChannelScalarDynamic &copy) :
  AnimChannelScalar(parent, copy),
  _value_node(copy._value_node),
  _value(copy._value),
  _last_value(nullptr),
  _value_changed(true),
  _float_value(copy._float_value)
{
}

/**
 *
 */
AnimChannelScalarDynamic::
AnimChannelScalarDynamic(const std::string &name)
  : AnimChannelScalar(name)
{
  _last_value = _value = TransformState::make_identity();
  _value_changed = true;
  _float_value = 0.0;
}

/**
 * Returns true if the value has changed since the last call to has_changed().
 * last_frame is the frame number of the last call; this_frame is the current
 * frame number.
 */
bool AnimChannelScalarDynamic::
has_changed(int, double, int, double) {
  if (_value_node != nullptr) {
    _value = _value_node->get_transform();
    bool has_changed = (_value != _last_value);
    _last_value = _value;
    return has_changed;

  } else {
    bool has_changed = _value_changed;
    _value_changed = false;
    return has_changed;
  }
}

/**
 * Gets the value of the channel at the indicated frame.
 */
void AnimChannelScalarDynamic::
get_value(int, PN_stdfloat &value) {
  value = get_value();
}

/**
 * Explicitly sets the value.  This will remove any node assigned via
 * set_value_node().
 */
void AnimChannelScalarDynamic::
set_value(PN_stdfloat value) {
  _float_value = value;
  _value_node.clear();
  _value_changed = true;
}

/**
 * Specifies a node whose transform will be queried each frame to implicitly
 * specify the transform of this joint.  This will override the values set by
 * set_value().
 */
void AnimChannelScalarDynamic::
set_value_node(PandaNode *value_node) {
  if (_value_node == nullptr) {
    _last_value = TransformState::make_pos(LVecBase3(_float_value, 0.0f, 0.0f));
  }

  _value_node = value_node;

  if (_value_node != nullptr) {
    _value = _value_node->get_transform();
  }
}

/**
 * Returns a copy of this object, and attaches it to the indicated parent
 * (which may be NULL only if this is an AnimBundle).  Intended to be called
 * by copy_subtree() only.
 */
AnimGroup *AnimChannelScalarDynamic::
make_copy(AnimGroup *parent) const {
  return new AnimChannelScalarDynamic(parent, *this);
}


/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void AnimChannelScalarDynamic::
write_datagram(BamWriter *manager, Datagram &dg) {
  AnimChannelScalar::write_datagram(manager, dg);
  manager->write_pointer(dg, _value_node);
  manager->write_pointer(dg, _value);
  dg.add_stdfloat(_float_value);
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int AnimChannelScalarDynamic::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = AnimChannelScalar::complete_pointers(p_list, manager);

  // Get the _value_node and _value pointers.
  _value_node = DCAST(PandaNode, p_list[pi++]);
  _value = DCAST(TransformState, p_list[pi++]);

  return pi;
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimChannelScalarDynamic::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannelScalar::fillin(scan, manager);

  // Read the _value_node and _value pointers.
  manager->read_pointer(scan);
  manager->read_pointer(scan);

  _float_value = scan.get_stdfloat();
}

/**
 * Factory method to generate a AnimChannelScalarDynamic object
 */
TypedWritable *AnimChannelScalarDynamic::
make_AnimChannelScalarDynamic(const FactoryParams &params) {
  AnimChannelScalarDynamic *me = new AnimChannelScalarDynamic;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate a AnimChannelScalarDynamic object
 */
void AnimChannelScalarDynamic::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimChannelScalarDynamic);
}
