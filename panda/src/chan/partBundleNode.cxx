// Filename: partBundleNode.cxx
// Created by:  drose (23Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifdef WIN32_VC
#include "chan_headers.h"
#endif

#pragma hdrstop

#ifndef WIN32_VC
#include "partBundleNode.h"
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#endif

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
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void PartBundleNode::
write_datagram(BamWriter *manager, Datagram &me)
{
  NamedNode::write_datagram(manager, me);
  manager->write_pointer(me, _bundle);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void PartBundleNode::
fillin(DatagramIterator& scan, BamReader* manager)
{
  NamedNode::fillin(scan, manager);
  manager->read_pointer(scan, this);
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundleNode::complete_pointers
//       Access: Public
//  Description: Takes in a vector of pointes to TypedWritable
//               objects that correspond to all the requests for
//               pointers that this object made to BamReader.
////////////////////////////////////////////////////////////////////
int PartBundleNode::
complete_pointers(vector_typedWritable &plist, BamReader* manager)
{
  int start = NamedNode::complete_pointers(plist, manager);
  _bundle = DCAST(PartBundle, plist[start]);
  //Let PartBundleNode tell the PartBundle about itselt
  _bundle->_node = this;
  return start+1;
}


