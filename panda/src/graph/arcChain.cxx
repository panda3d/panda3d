// Filename: arcChain.cxx
// Created by:  drose (05Jan01)
// 
////////////////////////////////////////////////////////////////////

#include "arcChain.h"
#include "node.h"
#include "namedNode.h"

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::compare_to
//       Access: Public
//  Description: Returns a number less than zero if this ArcChain
//               sorts before the other one, greater than zero if it
//               sorts after, or zero if they are equivalent.
//
//               Two ArcChains are considered equivalent if they
//               consist of exactly the same list of arcs in the same
//               order.  Otherwise, they are different; different
//               ArcChains will be ranked in a consistent but
//               undefined ordering; the ordering is useful only for
//               placing the ArcChains in a sorted container like an
//               STL set.
////////////////////////////////////////////////////////////////////
int ArcChain::
compare_to(const ArcChain &other) const {
  ArcComponent *a = _head;
  ArcComponent *b = other._head;

  while (a != (ArcComponent *)NULL && b != (ArcComponent *)NULL) {
    if (a < b) {
      return -1;
    } else if (a > b) {
      return 1;
    }

    a = a->_next;
    b = b->_next;
  }

  if (a < b) {
    return -1;
  } else if (a > b) {
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ArcChain::r_output
//       Access: Private
//  Description: The recursive implementation of output(), this writes
//               the names of each arc component in order from
//               beginning to end, by first walking to the end of the
//               linked list and then outputting from there.
////////////////////////////////////////////////////////////////////
void ArcChain::
r_output(ostream &out, ArcComponent *comp) const {
  ArcComponent *next = comp->_next;
  if (next != (ArcComponent *)NULL) {
    // This is not the head of the list; keep going up.
    r_output(out, next);
    out << "/";
  }

  // Now output this component.
  Node *node = comp->_arc->get_child();
  if (node->is_of_type(NamedNode::get_class_type())) {
    NamedNode *named_node = DCAST(NamedNode, node);
    if (named_node->has_name()) {
      out << named_node->get_name();
    } else {
      out << node->get_type();
    }
  } else {
    out << node->get_type();
  }
}
