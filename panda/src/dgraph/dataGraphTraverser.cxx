// Filename: dataGraphTraverser.cxx
// Created by:  drose (05Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "dataGraphTraverser.h"
#include "dataRelation.h"
#include "dataNode.h"
#include "config_dgraph.h"
#include "describe_data_verbose.h"



////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::r_traverse
//       Access: Private
//  Description: The recursive implementation of traverse().
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
r_traverse(Node *node, NodeAttributes &data, bool has_spam_mode) {
  DataNode *data_node = (DataNode *)NULL;
  if (node->is_of_type(DataNode::get_class_type())) {
    DCAST_INTO_V(data_node, node);

#ifdef NOTIFY_DEBUG
    has_spam_mode = has_spam_mode || data_node->get_spam_mode();
#endif
  }

  int num_parents = node->get_num_parents(DataRelation::get_class_type());

  if (num_parents > 1) {
    // This node is multiply parented, so we must save it for later.
    save(node, data, has_spam_mode, num_parents);

  } else {

    // Now ask the node to transmit its overall data.
    if (data_node != (DataNode *)NULL) {
#ifdef NOTIFY_DEBUG
      if (has_spam_mode && dgraph_cat.is_info()) {
	dgraph_cat.info() << "Sending into " << *node << " {\n";
	describe_data_verbose(dgraph_cat.info(false), data, 2);
	dgraph_cat.info(false) << "}\n";
	
      } else if (dgraph_cat.is_spam()) {
	dgraph_cat.spam() << "Sending into " << *node << " {\n";
	describe_data_verbose(dgraph_cat.spam(false), data, 2);
	dgraph_cat.spam(false) << "}\n";
      }
#endif

      data_node->transmit_data(data);
    }

    r_traverse_below(node, data, has_spam_mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::r_traverse_below
//       Access: Private
//  Description: Resumes the traversal below the indicated node, given
//               the output from a previous call to
//               Node::transmit_data().
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
r_traverse_below(Node *node, NodeAttributes &data, bool has_spam_mode) {
  DataNode *data_node = (DataNode *)NULL;
  if (node->is_of_type(DataNode::get_class_type())) {
    DCAST_INTO_V(data_node, node);

#ifdef NOTIFY_DEBUG
    has_spam_mode = has_spam_mode || data_node->get_spam_mode();
#endif
  }

  int num_children = node->get_num_children(DataRelation::get_class_type());

  if (num_children > 0) {
    // For the first n - 1 children we need to make a copy of our
    // NodeAttributes for each one--this allows our children to modify
    // the set freely without affecting its siblings.
  
    for (int i = 0; i < num_children - 1; i++) {
      NodeAttributes copy = data;
      if (data_node != (DataNode *)NULL) {
	data_node->transmit_data_per_child(copy, i);
      }
      
      NodeRelation *arc = 
	node->get_child(DataRelation::get_class_type(), i);
      r_traverse(arc->get_child(), copy, has_spam_mode);
    }
    
    // For the last child, we don't have to bother making a copy since
    // no one cares any more.  This is a slight optimization.
    if (data_node != (DataNode *)NULL) {
      data_node->transmit_data_per_child(data, num_children - 1);
    }
    
    NodeRelation *arc = 
      node->get_child(DataRelation::get_class_type(), num_children - 1);

    if (dgraph_cat.is_spam()) {
      dgraph_cat.spam() << "Traversing " << *arc << "\n";
    }

    r_traverse(arc->get_child(), data, has_spam_mode);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::save
//       Access: Private
//  Description: Saves the current state for later, until we have
//               extracted the data from all the parents of this node.
//               When the last parent is accounted for, resumes the
//               traversal for this node and removes it from the saved
//               queue.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
save(Node *node, const NodeAttributes &data, bool has_spam_mode,
     int num_parents) {
   
  States::iterator si;
  si = _saved_states.find(node);

  if (si != _saved_states.end()) {
    SavedState &state = (*si).second;
    state._data.merge_from(state._data, data);
    state._has_spam_mode = state._has_spam_mode || has_spam_mode;
    state._num_parents_so_far++;

    if (dgraph_cat.is_spam()) {
      dgraph_cat.spam()
	<< "Encountered " << *node << " with " << num_parents
	<< " parents; " << state._num_parents_so_far
	<< " processed so far.\n";
    }

    nassertv(state._num_parents_so_far <= num_parents);
    if (state._num_parents_so_far == num_parents) {
      // All parents are accounted for, so resume traversal!
      resume(node, state);
      _saved_states.erase(si);
    }
    return;
  }

  // This node has not been encountered before, so save an entry for
  // it.
  if (dgraph_cat.is_spam()) {
    dgraph_cat.spam()
      << "Encountered " << *node << " with " << num_parents
      << " parents.\n";
  }

  SavedState &state = _saved_states[node];
  state._data = data;
  state._has_spam_mode = has_spam_mode;
  state._num_parents_so_far = 1;
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::resume
//       Access: Private
//  Description: Resumes traversal from a previously-saved state.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
resume(Node *node, DataGraphTraverser::SavedState &state) {
  DataNode *data_node = (DataNode *)NULL;
  if (node->is_of_type(DataNode::get_class_type())) {
    DCAST_INTO_V(data_node, node);

#ifdef NOTIFY_DEBUG
    if (state._has_spam_mode && dgraph_cat.is_info()) {
      dgraph_cat.info() << "Sending into " << *node << " {\n";
      describe_data_verbose(dgraph_cat.info(false), state._data, 2);
      dgraph_cat.info(false) << "}\n";
      
    } else if (dgraph_cat.is_spam()) {
      dgraph_cat.spam() << "Sending into " << *node << " {\n";
      describe_data_verbose(dgraph_cat.spam(false), state._data, 2);
      dgraph_cat.spam(false) << "}\n";
    }
#endif
    data_node->transmit_data(state._data);
  }

  r_traverse_below(node, state._data, state._has_spam_mode);
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::resume_all
//       Access: Private
//  Description: Called after the traversal has completed, this
//               resumes all of the unresumed saved states.  There
//               shouldn't actually be any such things, unless a node
//               is multiply parented to a node outside of the data
//               graph.
////////////////////////////////////////////////////////////////////
void DataGraphTraverser::
resume_all() {
  while (!_saved_states.empty()) {
    States::iterator si = _saved_states.begin();
    Node *node = (*si).first;
    SavedState &state = (*si).second;

    dgraph_cat.warning()
      << *node << " is parented both inside and outside of the data graph:\n";
    int num_parents = node->get_num_parents(DataRelation::get_class_type());
    for (int i = 0; i < num_parents; i++) {
      NodeRelation *arc = node->get_parent(DataRelation::get_class_type(), i);
      dgraph_cat.warning(false)
	<< "  " << *arc << "\n";
    }

    resume(node, state);
    _saved_states.erase(si);
  }
}

