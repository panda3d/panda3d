// Filename: node.I
// Created by:  drose (26Oct98)
// 
////////////////////////////////////////////////////////////////////

#include <notify.h>


////////////////////////////////////////////////////////////////////
//     Function: remove_child
//  Description: Finds the arc that connects the indicated nodes and
//               removes it.  Returns true if the arc was found and
//               removed, false if it did not exist.
////////////////////////////////////////////////////////////////////
INLINE bool
remove_child(Node *parent, Node *child, TypeHandle graph_type) {
  NodeRelation *arc = find_arc(parent, child, graph_type);
  if (arc != (NodeRelation *)NULL) {
    remove_arc(arc);
    return true;
  }
  return false;
}
