// Filename: dataGraphTraverser.cxx
// Created by:  drose (11Mar02)
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

#include "dataGraphTraverser.h"
#include "dataNode.h"
#include "config_dgraph.h"
#include "dcast.h"


////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::CollectedData::set_data
//       Access: Public
//  Description: Sets the data associated with the indicated parent of
//               this CollectedData object's node.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::CollectedData::
set_data(int parent_index, const DataNodeTransmit &data) {
  if ((int)_data.size() <= parent_index) {
    _data.reserve(parent_index + 1);
    while ((int)_data.size() <= parent_index) {
      _data.push_back(DataNodeTransmit());
    }
  }

  nassertv(parent_index >= 0 && parent_index < (int)_data.size());
  _data[parent_index] = data;
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DataGraphTraverser::
DataGraphTraverser() {
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DataGraphTraverser::
~DataGraphTraverser() {
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::traverse
//       Access: Public
//  Description: Starts the traversal of the data graph at the
//               indicated root node.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
traverse(PandaNode *node) {
  if (node->is_of_type(DataNode::get_class_type())) {
    DataNode *data_node = DCAST(DataNode, node);
    int num_parents = data_node->get_num_parents();
    // We must start the traversal at the root of the graph.
    nassertv(num_parents == 0);

    r_transmit(data_node, (DataNodeTransmit *)NULL);

  } else {
    traverse_below(node, DataNodeTransmit());
  }

  collect_leftovers();
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::traverse_below
//       Access: Public
//  Description: Continues the traversal to all the children of the
//               indicated node, passing in the given data, without
//               actually calling transmit_data() on the given node.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
traverse_below(PandaNode *node, const DataNodeTransmit &output) {
  PandaNode::Children cr = node->get_children();
  int num_children = cr.get_num_children();

  for (int i = 0; i < num_children; i++) {
    PandaNode *child_node = cr.get_child(i);
    if (child_node->is_of_type(DataNode::get_class_type())) {
      DataNode *data_node = DCAST(DataNode, child_node);
      // If it's a DataNode-type child, we need to pass it the data.
      // Maybe it has only one parent, and can accept the data
      // immediately.
      int num_parents = data_node->get_num_parents();
      if (num_parents == 1) {
        // The easy, common case: only one parent.  We make our output
        // into a one-element array of inputs by turning it into a
        // pointer.
        r_transmit(data_node, &output);
      } else {
        // A more difficult case: multiple parents.  We must collect
        // instances together, meaning we must hold onto this node
        // until we have reached it through all paths.
        CollectedData &collected_data = _multipass_data[data_node];
        int parent_index = data_node->find_parent(node);
        nassertv(parent_index != -1);

        collected_data.set_data(parent_index, output);
        collected_data._num_parents++;
        nassertv(collected_data._num_parents <= num_parents);
        if (collected_data._num_parents == num_parents) {
          // Now we've got all the data; go into the child.
          r_transmit(data_node, &collected_data._data[0]);
          _multipass_data.erase(data_node);
        }
      }
    } else {
      // The child node is not a DataNode-type child.  We continue the
      // traversal, but data does not pass through this node, and
      // instances are not collected together.  (Although we appear to
      // be passing the data through here, it doesn't do any good
      // anyway, since the child nodes of this node will not know how
      // to interpret the data from a non-DataNode parent.)
      traverse_below(child_node, output);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::collect_leftovers
//       Access: Public
//  Description: Pick up any nodes that didn't get completely
//               traversed.  These must be nodes that have multiple
//               parents, with at least one parent completely outside
//               of the data graph.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
collect_leftovers() {
  while (!_multipass_data.empty()) {
    MultipassData::iterator mi = _multipass_data.begin();
    DataNode *data_node = (*mi).first;
    const CollectedData &collected_data = (*mi).second;

    dgraph_cat.warning()
      << *data_node << " improperly parented partly outside of data graph.\n";

    r_transmit(data_node, &collected_data._data[0]);
    _multipass_data.erase(mi);
  }  
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::r_transmit
//       Access: Private
//  Description: Part of the recursive implementation of traverse().
//               This transmits the given data into the indicated
//               DataNode, and then sends the output data to each of
//               the node's children.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
r_transmit(DataNode *data_node, const DataNodeTransmit inputs[]) {
  DataNodeTransmit output;
  output.reserve(data_node->get_num_outputs());
  data_node->transmit_data(inputs, output);

  traverse_below(data_node, output);
}
