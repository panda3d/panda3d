// Filename: modelNode.I
// Created by:  drose (09Nov00)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: ModelNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ModelNode::
ModelNode(const string &name) : 
  NamedNode(name) 
{ 
  _preserve_transform = false;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::set_preserve_transform
//       Access: Public
//  Description: Sets the preserve_transform flag.  When this flag is
//               true, flattening the scene graph will not flatten out
//               any transformation assigned above this node;
//               otherwise, any transforms applying to this node may
//               or may not be flattened.
////////////////////////////////////////////////////////////////////
INLINE void ModelNode::
set_preserve_transform(bool preserve_transform) {
  _preserve_transform = preserve_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::get_preserve_transform
//       Access: Public
//  Description: Returns the current setting of the preserve_transform
//               flag.  See set_preserve_transform().
////////////////////////////////////////////////////////////////////
INLINE bool ModelNode::
get_preserve_transform() const {
  return _preserve_transform;
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE ModelNode::
ModelNode(const ModelNode &copy) :
  NamedNode(copy),
  _preserve_transform(copy._preserve_transform)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ModelNode::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void ModelNode::
operator = (const ModelNode &copy) {
  NamedNode::operator = (copy);
  _preserve_transform = copy._preserve_transform;
}
