// Filename: graphReducer.cxx
// Created by:  drose (26Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "graphReducer.h"
#include "config_graph.h"
#include "namedNode.h"
#include "pt_Node.h"

#include <map>
#include <list>

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphReducer::
GraphReducer(TypeHandle graph_type) :
  _graph_type(graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
GraphReducer::
~GraphReducer() {
}


////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::flatten
//       Access: Public
//  Description: Simplifies the graph by removing unnecessary nodes
//               and arcs.
//
//               In general, a node (and its parent arc) is a
//               candidate for removal if the node has no siblings and
//               the node and arc have no special properties.  The
//               definition of what, precisely, is a 'special
//               property' may be extended by subclassing from this
//               type and redefining consider_arc() appropriately.
//
//               If combine_siblings is true, sibling nodes may also
//               be collapsed into a single node.  This will further
//               reduce scene graph complexity, sometimes
//               substantially, at the cost of reduced spatial
//               separation.
//
//               Returns the number of arcs removed from the graph.
////////////////////////////////////////////////////////////////////
int GraphReducer::
flatten(Node *root, bool combine_siblings) {
  int num_total_nodes = 0;
  int num_pass_nodes;

  do {
    num_pass_nodes = 0;

    const DownRelationPointers &drp = 
      root->find_connection(_graph_type).get_down();
      
    // Get a copy of the children list, so we don't have to worry
    // about self-modifications.
    DownRelationPointers drp_copy = drp;
      
    // Now visit each of the children in turn.
    DownRelationPointers::const_iterator drpi;
    for (drpi = drp_copy.begin(); drpi != drp_copy.end(); ++drpi) {
      NodeRelation *arc = (*drpi);
      num_pass_nodes += r_flatten(arc->get_child(), combine_siblings);
    }

    num_total_nodes += num_pass_nodes;

    // If combine_siblings is true, we should repeat the above until
    // we don't get any more benefit from flattening, because each
    // pass could convert cousins into siblings, which may get
    // flattened next pass.  If combine_siblings is not true, the
    // first pass will be fully effective, and there's no point in
    // trying again.
  } while (combine_siblings && num_pass_nodes != 0);

  return num_total_nodes;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::r_flatten
//       Access: Protected
//  Description: The recursive implementation of flatten().
////////////////////////////////////////////////////////////////////
int GraphReducer::
r_flatten(Node *root, bool combine_siblings) {
  int num_nodes = 0;

  const DownRelationPointers &drp = 
    root->find_connection(_graph_type).get_down();

  // Get a copy of the children list, so we don't have to worry
  // about self-modifications.
  DownRelationPointers drp_copy = drp;

  // Now visit each of the children in turn.
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp_copy.begin(); drpi != drp_copy.end(); ++drpi) {
    NodeRelation *arc = (*drpi);
    num_nodes += r_flatten(arc->get_child(), combine_siblings);
  }
  
  if (combine_siblings && drp.size() >= 2) {
    num_nodes += flatten_siblings(root);
  }
  
  if (drp.size() == 1) {
    // If we have exactly one child, consider flattening it.
    NodeRelation *arc = *drp.begin();
    if (consider_arc(arc)) {
      if (flatten_arc(arc)) {
        num_nodes++;
      }
    }
  }

  return num_nodes;
}

class SortByTransitions {
public:
  INLINE bool 
  operator () (const NodeRelation *arc1, const NodeRelation *arc2) const;
};

INLINE bool SortByTransitions::
operator () (const NodeRelation *arc1, const NodeRelation *arc2) const {
  return (arc1->compare_transitions_to(arc2) < 0);
}


////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::flatten_siblings
//       Access: Protected
//  Description: Attempts to collapse together any pairs of siblings
//               of the indicated node that share the same properties.
////////////////////////////////////////////////////////////////////
int GraphReducer::
flatten_siblings(Node *root) {
  int num_nodes = 0;

  // First, collect the children into groups of arcs with common
  // properties.
  typedef map<NodeRelation *, list<NodeRelation *>, SortByTransitions> Children;
  Children children;

  const DownRelationPointers &drp = 
    root->find_connection(_graph_type).get_down();
  DownRelationPointers::const_iterator drpi;
  for (drpi = drp.begin(); drpi != drp.end(); ++drpi) {
    NodeRelation *arc = (*drpi);
    children[arc].push_back(arc);
  }

  // Now visit each of those groups and try to collapse them together.
  Children::iterator ci;
  for (ci = children.begin(); ci != children.end(); ++ci) {
    list<NodeRelation *> &arcs = (*ci).second;

    list<NodeRelation *>::iterator ai1;
    ai1 = arcs.begin();
    while (ai1 != arcs.end()) {
      list<NodeRelation *>::iterator ai1_hold = ai1;
      NodeRelation *arc1 = (*ai1);
      ++ai1;
      list<NodeRelation *>::iterator ai2 = ai1;
      while (ai2 != arcs.end()) {
        list<NodeRelation *>::iterator ai2_hold = ai2;
        NodeRelation *arc2 = (*ai2);
        ++ai2;

        if (consider_siblings(root, arc1, arc2)) {
          NodeRelation *new_arc = collapse_siblings(root, arc1, arc2);
          if (new_arc != (NodeRelation *)NULL) {
            // We successfully collapsed an arc.
            arcs.erase(ai2_hold);
            arcs.erase(ai1_hold);
            arcs.push_back(new_arc);
            ai1 = arcs.begin();
            ai2 = arcs.end();
            num_nodes++;
          }
        }
      }
    }
  }

  return num_nodes;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::consider_arc
//       Access: Protected, Virtual
//  Description: Decides whether or not the indicated arc is a
//               suitable candidate for removal.  Returns true if the
//               arc may be removed, false if it should be kept.  This
//               function may be extended in a user class to protect
//               special kinds of arcs from deletion.
////////////////////////////////////////////////////////////////////
bool GraphReducer::
consider_arc(NodeRelation *arc) {
  if (arc->has_sub_render_trans()) {
    if (graph_cat.is_debug()) {
      graph_cat.debug()
        << "Not removing " << *arc
        << " because it contains a sub_render transition.\n";
    }
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::consider_siblings
//       Access: Protected, Virtual
//  Description: Decides whether or not the indicated sibling nodes
//               (and their associated arcs) should be collapsed into
//               a single node or not.  Returns true if the arcs may
//               be collapsed, false if they should be kept distinct.
////////////////////////////////////////////////////////////////////
bool GraphReducer::
consider_siblings(Node *, NodeRelation *arc1, NodeRelation *arc2) {
  // Don't attempt to combine any sibling arcs with different
  // transitions.
  return (arc1->compare_transitions_to(arc2) == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::flatten_arc
//       Access: Protected, Virtual
//  Description: Removes the indicated arc, collapsing together the
//               two nodes and leaving them parented in the same
//               place.  The return value is true if the arc is
//               successfully collapsed, false if we chickened out.
//
//               This function may be extended in a user class to
//               handle special kinds of nodes.
////////////////////////////////////////////////////////////////////
bool GraphReducer::
flatten_arc(NodeRelation *arc) {
  PT_Node parent = arc->get_parent();
  PT_Node child = arc->get_child();

  if (graph_cat.is_debug()) {
    graph_cat.debug()
      << "Removing " << *arc << "\n";
  }

  PT_Node new_parent = collapse_nodes(parent, child, false);
  if (new_parent == (Node *)NULL) {
    if (graph_cat.is_debug()) {
      graph_cat.debug()
        << "Decided not to remove " << *arc << "\n";
    }
    return false;
  }

  choose_name(new_parent, parent, child);

  move_children(new_parent, parent);
  move_children(new_parent, child);

  // Move all of the transitions from the arc to the parent's arcs.
  int num_grandparents = parent->get_num_parents(_graph_type);
  for (int i = 0; i < num_grandparents; i++) {
    NodeRelation *grandparent_arc = parent->get_parent(_graph_type, i);

    grandparent_arc->compose_transitions_from(arc);
    if (new_parent != parent) {
      // Also switch out the parent node.
      grandparent_arc->change_child(new_parent);
    }
  }

  remove_arc(arc);  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::collapse_siblings
//       Access: Protected, Virtual
//  Description: Performs the work of collapsing two sibling arcs
//               together into a single arc (and their associated
//               nodes into a single node).  
//
//               Returns a pointer to a NodeRelation the reflects the
//               combined arc (which may be either of the source arcs,
//               or a new arc altogether) if the siblings are
//               successfully collapsed, or NULL if we chickened out.
////////////////////////////////////////////////////////////////////
NodeRelation *GraphReducer::
collapse_siblings(Node *parent, NodeRelation *arc1, NodeRelation *arc2) {
  PT_Node node1 = arc1->get_child();
  PT_Node node2 = arc2->get_child();

  PT_Node new_node = collapse_nodes(node1, node2, true);
  if (new_node == (Node *)NULL) {
    graph_cat.debug()
      << "Decided not to collapse " << *node1 << " and " << *node2 << "\n";
    return NULL;
  }

  move_children(new_node, node1);
  move_children(new_node, node2);

  arc1->change_parent_and_child(parent, node1);
  remove_arc(arc2);

  return arc1;
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::collapse_nodes
//       Access: Protected, Virtual
//  Description: Collapses the two nodes into a single node, if
//               possible.  The 'siblings' flag is true if the two
//               nodes are siblings nodes; otherwise, node1 is a
//               parent of node2.  The return value is the resulting
//               node, which may be either one of the source nodes, or
//               a new node altogether, or it may be NULL to indicate
//               that the collapse operation could not take place.
//
//               This function may be extended in a user class to
//               handle combining special kinds of nodes.
////////////////////////////////////////////////////////////////////
Node *GraphReducer::
collapse_nodes(Node *node1, Node *node2, bool) {
  // We get to choose whether to remove node1 or node2.
  if (node2->is_exact_type(Node::get_class_type()) ||
      node2->is_exact_type(NamedNode::get_class_type())) {
    // Node2 isn't anything special, so preserve node1.
    return node1;
    
  } else if (node1->is_exact_type(Node::get_class_type()) ||
             node1->is_exact_type(NamedNode::get_class_type())) {
    // Node1 isn't anything special, so preserve node2.
    return node2;

  } else {
    // Both node1 and node2 are some special kind of node.  Don't
    // want to risk removing either of them.
    return NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::choose_name
//       Access: Protected, Virtual
//  Description: Chooses a suitable name for the collapsed node, based
//               on the names of the two sources nodes.
////////////////////////////////////////////////////////////////////
void GraphReducer::
choose_name(Node *preserve, Node *source1, Node *source2) {
  if (!preserve->is_of_type(NamedNode::get_class_type())) {
    // It's not even a namable class, so we can't name it anyway.
    // Never mind.
    return;
  }

  NamedNode *named_preserve;
  DCAST_INTO_V(named_preserve, preserve);

  if (source1->is_of_type(NamedNode::get_class_type())) {
    NamedNode *named_source1;
    DCAST_INTO_V(named_source1, source1);

    if (!named_source1->get_name().empty()) {
      named_preserve->set_name(named_source1->get_name());
      return;
    }
  }

  if (source2->is_of_type(NamedNode::get_class_type())) {
    NamedNode *named_source2;
    DCAST_INTO_V(named_source2, source2);

    if (!named_source2->get_name().empty()) {
      named_preserve->set_name(named_source2->get_name());
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::move_children
//       Access: Protected
//  Description: Moves all of the children arcs of the 'from' node to
//               the 'to' node.
////////////////////////////////////////////////////////////////////
void GraphReducer::
move_children(Node *to, Node *from) {
  if (to != from) {
    int num_children = from->get_num_children(_graph_type);
    while (num_children > 0) {
      NodeRelation *arc = 
        from->get_child(_graph_type, 0);
      arc->change_parent(to);
      num_children--;
      nassertv(num_children == from->get_num_children(_graph_type));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GraphReducer::copy_children
//       Access: Protected
//  Description: Copies all of the children arcs of the 'from' node to
//               the 'to' node, without removing the from the 'from'
//               node.
////////////////////////////////////////////////////////////////////
void GraphReducer::
copy_children(Node *to, Node *from) {
  if (to != from) {
    int num_children = from->get_num_children(_graph_type);
    for (int i = 0; i < num_children; i++) {
      NodeRelation *arc = from->get_child(_graph_type, i);
      NodeRelation *new_arc = NodeRelation::create_typed_arc
        (_graph_type, to, arc->get_child());
      nassertv(new_arc != (NodeRelation *)NULL);
      nassertv(new_arc->is_exact_type(_graph_type));
      new_arc->copy_transitions_from(arc);
    }
  }
}
