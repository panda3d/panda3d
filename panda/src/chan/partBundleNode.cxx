// Filename: partBundleNode.cxx
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "partBundleNode.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle PartBundleNode::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::safe_to_flatten
//       Access: Public, Virtual
//  Description: Returns true if it is generally safe to flatten out
//               this particular kind of Node by duplicating
//               instances, false otherwise (for instance, a Camera
//               cannot be safely flattened, because the Camera
//               pointer itself is meaningful).
////////////////////////////////////////////////////////////////////
bool PartBundleNode::
safe_to_flatten() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void PartBundleNode::
write_datagram(BamWriter *manager, Datagram &dg) {
  PandaNode::write_datagram(manager, dg);
  manager->write_pointer(dg, _bundle);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int PartBundleNode::
complete_pointers(TypedWritable **p_list, BamReader* manager) {
  int pi = PandaNode::complete_pointers(p_list, manager);
  _bundle = DCAST(PartBundle, p_list[pi++]);
  _bundle->_node = this;
  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new PandaNode.
////////////////////////////////////////////////////////////////////
void PartBundleNode::
fillin(DatagramIterator &scan, BamReader* manager) {
  PandaNode::fillin(scan, manager);
  manager->read_pointer(scan);
}
