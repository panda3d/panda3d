// Filename: partBundleNode.I
// Created by:  drose (23Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::Constructor
//       Access: Public
//  Description: The PartBundle and its node should be constructed
//               together.  Generally, the derived classes of
//               PartBundleNode will automatically create a PartBundle
//               of the appropriate type, and pass it up to this
//               constructor.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode::
PartBundleNode(const string &name, PartBundle *bundle) :
  NamedNode(name), 
  _bundle(bundle) 
{
  _bundle->_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::Default Constructor
//       Access: Protected
//  Description: For internal use only.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode::
PartBundleNode(void)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::Copy Constructor
//       Access: Protected
//  Description: Use make_copy() or copy_subgraph() to copy one of
//               these.  Copying a PartBundleNode will always force a
//               deep copy of the PartGroup hierarchy.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode::
PartBundleNode(const PartBundleNode &copy) :
  NamedNode(copy),
  _bundle(DCAST(PartBundle, copy._bundle->copy_subgraph()))
{
  _bundle->_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::get_bundle
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PartBundle *PartBundleNode::
get_bundle() const {
  return _bundle;
}
