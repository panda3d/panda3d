// Filename: eggExternalReference.I
// Created by:  drose (11Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggExternalReference::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggExternalReference &EggExternalReference::
operator = (const string &filename) {
  EggFilenameNode::operator = (filename);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggExternalReference::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggExternalReference &EggExternalReference::
operator = (const char *filename) {
  EggFilenameNode::operator = (filename);
  return *this;
}

////////////////////////////////////////////////////////////////////
//     Function: EggExternalReference::Assignment operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE EggExternalReference &EggExternalReference::
operator = (const Filename &filename) {
  EggFilenameNode::operator = (filename);
  return *this;
}
