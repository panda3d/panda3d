// Filename: eggFilenameNode.cxx
// Created by:  drose (11Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "eggFilenameNode.h"

TypeHandle EggFilenameNode::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: EggFilenameNode::get_default_extension
//       Access: Public, Virtual
//  Description: Returns the default extension for this filename type.
////////////////////////////////////////////////////////////////////
string EggFilenameNode::
get_default_extension() const {
  return string();
}
