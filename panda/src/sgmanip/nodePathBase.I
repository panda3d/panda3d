// Filename: nodePathBase.I
// Created by:  drose (06Mar00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: NodePathBase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodePathBase::
NodePathBase(TypeHandle graph_type) :
  _graph_type(graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathBase::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodePathBase::
NodePathBase(Node *top_node, TypeHandle graph_type) :
  ArcChain(top_node),
  _graph_type(graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathBase::Constructor
//       Access: Public
//  Description: This constructor creates a new NodePathBase given an
//               ArcChain, which is a more primitive version of a
//               NodePath.  Some graph-traversal operations generate
//               ArcChains, so it's useful to be able to use one of
//               these to create a NodePath.
////////////////////////////////////////////////////////////////////
INLINE NodePathBase::
NodePathBase(const ArcChain &chain, TypeHandle graph_type) :
  ArcChain(chain),
  _graph_type(graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathBase::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE NodePathBase::
NodePathBase(const NodePathBase &copy) :
  ArcChain(copy),
  _graph_type(copy._graph_type)
{
}

////////////////////////////////////////////////////////////////////
//     Function: NodePathBase::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void NodePathBase::
operator = (const NodePathBase &copy) {
  ArcChain::operator = (copy);
  _graph_type = copy._graph_type;
}
