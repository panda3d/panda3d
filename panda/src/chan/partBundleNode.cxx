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
//     Function: PartBundleNode::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PartBundleNode::
~PartBundleNode() {
  Bundles::iterator bi;
  for (bi = _bundles.begin(); bi != _bundles.end(); ++bi) {
    nassertv((*bi)->_node == this);
    (*bi)->_node = NULL;
  }
}

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
//     Function: PartBundleNode::xform
//       Access: Public, Virtual
//  Description: Transforms the contents of this PandaNode by the
//               indicated matrix, if it means anything to do so.  For
//               most kinds of PandaNodes, this does nothing.
////////////////////////////////////////////////////////////////////
void PartBundleNode::
xform(const LMatrix4f &mat) {
  Bundles::iterator bi;
  for (bi = _bundles.begin(); bi != _bundles.end(); ++bi) {
    (*bi)->xform(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::add_bundle
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
void PartBundleNode::
add_bundle(PartBundle *bundle) {
  nassertv(bundle->_node == NULL);
  _bundles.push_back(bundle);
  bundle->_node = this;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::steal_bundles
//       Access: Protected
//  Description: Moves the PartBundles from the other node onto this
//               one.
////////////////////////////////////////////////////////////////////
void PartBundleNode::
steal_bundles(PartBundleNode *other) {
  Bundles::iterator bi;
  for (bi = other->_bundles.begin(); bi != other->_bundles.end(); ++bi) {
    PartBundle *bundle = (*bi);
    _bundles.push_back(bundle);
    bundle->_node = this;
  }
  other->_bundles.clear();
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

  dg.add_uint16(_bundles.size());
  Bundles::iterator bi;
  for (bi = _bundles.begin(); bi != _bundles.end(); ++bi) {
    manager->write_pointer(dg, (*bi));
  }
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

  Bundles::iterator bi;
  for (bi = _bundles.begin(); bi != _bundles.end(); ++bi) {
    (*bi) = DCAST(PartBundle, p_list[pi++]);
    (*bi)->_node = this;
  }

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

  int num_bundles = 1;
  if (manager->get_file_minor_ver() >= 5) {
    num_bundles = scan.get_uint16();
  }

  for (int i = 0; i < num_bundles; ++i) {
    manager->read_pointer(scan);
  }
}
