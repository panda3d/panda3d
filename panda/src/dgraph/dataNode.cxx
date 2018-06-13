/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file dataNode.cxx
 * @author drose
 * @date 2002-03-11
 */

#include "dataNode.h"
#include "dataNodeTransmit.h"
#include "config_dgraph.h"
#include "dcast.h"

using std::string;

TypeHandle DataNode::_type_handle;

/**
 * Returns a newly-allocated Node that is a shallow copy of this one.  It will
 * be a different Node pointer, but its internal data may or may not be shared
 * with that of the original Node.
 */
PandaNode *DataNode::
make_copy() const {
  return new DataNode(*this);
}

/**
 * Collects the data from all of the parent nodes and puts it into one
 * DataNodeTransmit object, for processing; calls do_transmit_data() to read
 * all the inputs and put the result into the indicated output.
 */
void DataNode::
transmit_data(DataGraphTraverser *trav,
              const DataNodeTransmit inputs[],
              DataNodeTransmit &output) {
  DataNodeTransmit new_input;
  new_input.reserve(get_num_inputs());

  DataConnections::const_iterator ci;
  for (ci = _data_connections.begin(); ci != _data_connections.end(); ++ci) {
    const DataConnection &connect = (*ci);
    const EventParameter &data =
      inputs[connect._parent_index].get_data(connect._output_index);
    if (!data.is_empty()) {
      new_input.set_data(connect._input_index, data);
    }
  }

  #ifndef NDEBUG
  if (dgraph_cat.is_spam()) {
    bool any_data = false;
    Wires::const_iterator wi;
    for (wi = _input_wires.begin(); wi != _input_wires.end(); ++wi) {
      const string &name = (*wi).first;
      const WireDef &def = (*wi).second;
      if (new_input.has_data(def._index)) {
        if (!any_data) {
          dgraph_cat.spam()
            << *this << " receives:\n";
          any_data = true;
        }
        dgraph_cat.spam(false)
          << "  " << name << " = " << new_input.get_data(def._index)
          << "\n";
      }
    }
  }
  #endif  // NDEBUG

  do_transmit_data(trav, new_input, output);

  #ifndef NDEBUG
  if (dgraph_cat.is_spam()) {
    bool any_data = false;
    Wires::const_iterator wi;
    for (wi = _output_wires.begin(); wi != _output_wires.end(); ++wi) {
      const string &name = (*wi).first;
      const WireDef &def = (*wi).second;
      if (output.has_data(def._index)) {
        if (!any_data) {
          dgraph_cat.spam()
            << *this << " transmits:\n";
          any_data = true;
        }
        dgraph_cat.spam(false)
          << "  " << name << " = " << output.get_data(def._index)
          << "\n";
      }
    }
  }
  #endif  // NDEBUG
}

/**
 * Writes to the indicated ostream a list of all the inputs this DataNode
 * might expect to receive.
 */
void DataNode::
write_inputs(std::ostream &out) const {
  Wires::const_iterator wi;
  for (wi = _input_wires.begin(); wi != _input_wires.end(); ++wi) {
    const string &name = (*wi).first;
    const WireDef &def = (*wi).second;
    out << name << " " << def._data_type << "\n";
  }
}

/**
 * Writes to the indicated ostream a list of all the outputs this DataNode
 * might generate.
 */
void DataNode::
write_outputs(std::ostream &out) const {
  Wires::const_iterator wi;
  for (wi = _output_wires.begin(); wi != _output_wires.end(); ++wi) {
    const string &name = (*wi).first;
    const WireDef &def = (*wi).second;
    out << name << " " << def._data_type << "\n";
  }
}

/**
 * Writes to the indicated ostream a list of all the connections currently
 * showing between this DataNode and its parent(s).
 */
void DataNode::
write_connections(std::ostream &out) const {
  DataConnections::const_iterator ci;
  for (ci = _data_connections.begin(); ci != _data_connections.end(); ++ci) {
    const DataConnection &connect = (*ci);
    nassertv(connect._parent_index >= 0 && connect._parent_index < get_num_parents());

    // Now we have to search exhaustively for the input with the matching
    // index number.
    Wires::const_iterator wi;
    bool found = false;
    for (wi = _input_wires.begin(); wi != _input_wires.end() && !found; ++wi) {
      const string &name = (*wi).first;
      const WireDef &def = (*wi).second;
      if (def._index == connect._input_index) {
        out << name << " " << def._data_type << " from "
            << *get_parent(connect._parent_index) << "\n";
        found = true;
      }
    }
    nassertv(found);
  }
}

/**
 * Adds a new input wire with the given name and the indicated data type.  The
 * data type should be the TypeHandle for some type that derives from
 * TypedReferenceCount, e.g.  EventStoreInt, EventStoreDouble, or some fancier
 * data type like Texture.
 *
 * If there is already an input wire defined with the indicated name, its type
 * is changed.
 *
 * The return value is the index into the "input" parameter to
 * do_transmit_data() that can be used to access the input data.
 */
int DataNode::
define_input(const string &name, TypeHandle data_type) {
  // We shouldn't already be connected.
  nassertr(_data_connections.empty(), 0);

  Wires::iterator wi;
  wi = _input_wires.find(name);
  if (wi != _input_wires.end()) {
    // This wire already existed; modify it and return the original index.
    WireDef &def = (*wi).second;
    def._data_type = data_type;
    return def._index;
  }

  // This wire did not already exist; add it.
  WireDef &def = _input_wires[name];
  def._data_type = data_type;
  def._index = _input_wires.size() - 1;
  return def._index;
}

/**
 * Adds a new output wire with the given name and the indicated data type.
 * The data type should be the TypeHandle for some type that derives from
 * TypedReferenceCount, e.g.  EventStoreInt, EventStoreDouble, or some fancier
 * data type like Texture.
 *
 * If there is already an output wire defined with the indicated name, its
 * type is changed.
 *
 * The return value is the index into the "output" parameter to
 * do_transmit_data() where the output data should be stored.
 */
int DataNode::
define_output(const string &name, TypeHandle data_type) {
  // We shouldn't already be connected.
  nassertr(_data_connections.empty(), 0);

  Wires::iterator wi;
  wi = _output_wires.find(name);
  if (wi != _output_wires.end()) {
    // This wire already existed; modify it and return the original index.
    WireDef &def = (*wi).second;
    def._data_type = data_type;
    return def._index;
  }

  // This wire did not already exist; add it.
  WireDef &def = _output_wires[name];
  def._data_type = data_type;
  def._index = _output_wires.size() - 1;
  return def._index;
}

/**
 * Called after a scene graph update that either adds or remove parents from
 * this node, this just provides a hook for derived PandaNode objects that
 * need to update themselves based on the set of parents the node has.
 */
void DataNode::
parents_changed() {
  PandaNode::parents_changed();
  reconnect();
}

/**
 * The virtual implementation of transmit_data().  This function receives an
 * array of input parameters and should generate an array of output
 * parameters.  The input parameters may be accessed with the index numbers
 * returned by the define_input() calls that were made earlier (presumably in
 * the constructor); likewise, the output parameters should be set with the
 * index numbers returned by the define_output() calls.
 */
void DataNode::
do_transmit_data(DataGraphTraverser *, const DataNodeTransmit &,
                 DataNodeTransmit &) {
}

/**
 * Establishes the input(s) that this DataNode has in common with its parents'
 * output(s).  Builds up the _data_connections list correspondingly.
 */
void DataNode::
reconnect() {
  int num_parents = get_num_parents();
  _data_connections.clear();
  // Look for each input among one of the parents.
  int num_datanode_parents = 0;

  Wires::const_iterator wi;
  for (wi = _input_wires.begin(); wi != _input_wires.end(); ++wi) {
    const string &name = (*wi).first;
    const WireDef &input_def = (*wi).second;

    int num_found = 0;
    for (int i = 0; i < num_parents; i++) {
      PandaNode *parent_node = get_parent(i);
      if (parent_node->is_of_type(DataNode::get_class_type())) {
        DataNode *data_node = DCAST(DataNode, parent_node);
        num_datanode_parents++;
        Wires::const_iterator pi;
        pi = data_node->_output_wires.find(name);
        if (pi != data_node->_output_wires.end()) {
          const WireDef &output_def = (*pi).second;
          num_found++;
          if (output_def._data_type != input_def._data_type) {
            dgraph_cat.warning()
              << "Ignoring mismatched type for connection " << name
              << " between " << *data_node << " and " << *this << "\n";
          } else {
            DataConnection dc;
            dc._parent_index = i;
            dc._output_index = output_def._index;
            dc._input_index = input_def._index;
            _data_connections.push_back(dc);
          }
        }
      }
    }

    if (num_found > 1) {
      if (dgraph_cat.is_debug()) {
        dgraph_cat.debug()
          << "Multiple connections found for " << name << " into " << *this
          << "\n";
      }
    }
  }

  if (_data_connections.empty() && get_num_inputs() != 0 &&
      num_datanode_parents != 0) {
    dgraph_cat.warning()
      << "No data connected to " << *this << "\n";
  }
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void DataNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
}

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new Lens.
 */
void DataNode::
fillin(DatagramIterator &scan, BamReader *manager) {
  PandaNode::fillin(scan, manager);
}
