// Filename: datagramGenerator.cxx
// Created by:  jason (07Jun00)
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


#include "pandabase.h"

#include "datagramGenerator.h"

////////////////////////////////////////////////////////////////////
//     Function: DatagramGenerator::Destructor
//       Access: Public, Virtual
//  Description: Does nothing since this is class is just
//               the definition of an interface
////////////////////////////////////////////////////////////////////
DatagramGenerator::
~DatagramGenerator() {
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGenerator::get_file
//       Access: Public, Virtual
//  Description: Returns the VirtualFile that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a VirtualFile.
////////////////////////////////////////////////////////////////////
VirtualFile *DatagramGenerator::
get_file() {
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramGenerator::get_file_pos
//       Access: Public, Virtual
//  Description: Returns the current file position within the data
//               stream, if any, or 0 if the file position is not
//               meaningful or cannot be determined.
//
//               For DatagramGenerators that return a meaningful file
//               position, this will be pointing to the first byte
//               following the datagram returned after a call to
//               get_datagram().
////////////////////////////////////////////////////////////////////
streampos DatagramGenerator::
get_file_pos() {
  return 0;
}
