// Filename: dataGraphTraverser.I
// Created by:  drose (05Feb01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE DataGraphTraverser::
DataGraphTraverser() {
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::traverse
//       Access: Public
//  Description: Fires off the traversal, beginning at the indicated
//               node.
////////////////////////////////////////////////////////////////////
INLINE void DataGraphTraverser::
traverse(Node *node) {
  NodeAttributes empty;
  r_traverse(node, empty, false);
  resume_all();
}

////////////////////////////////////////////////////////////////////
//     Function: DataGraphTraverser::traverse_below
//       Access: Public
//  Description: Fires off the traversal, beginning *below* the
//               indicated node.  The transmit_data() function is not
//               called on the starting node; instead, the indicated
//               data attributes are taken as the output of that node.
//
//               If the node defines transmit_data_per_child(), that
//               function is still called, once for each child as
//               usual.
////////////////////////////////////////////////////////////////////
INLINE void DataGraphTraverser::
traverse_below(Node *node, const NodeAttributes &data) {
  NodeAttributes copy = data;
  r_traverse_below(node, copy, false);
  resume_all();
}
