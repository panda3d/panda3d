// Filename: appTraverser.cxx
// Created by:  drose (25Apr00)
// 
////////////////////////////////////////////////////////////////////

#include "appTraverser.h"
#include "config_sgraphutil.h"

#include <dftraverser.h>

////////////////////////////////////////////////////////////////////
//     Function: AppTraverser::traverse
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void AppTraverser::
traverse(Node *root) {
  if (!implicit_app_traversal) {
    df_traverse(root, *this, NullAttributeWrapper(), NullLevelState(), 
		_graph_type);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AppTraverser::reached_node 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool AppTraverser::
reached_node(Node *node, NullAttributeWrapper &, NullLevelState &) {
  node->app_traverse();

  return true;
}
