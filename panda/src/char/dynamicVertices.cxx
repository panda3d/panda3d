// Filename: dynamicVertices.cxx
// Created by:  drose (01Mar99)
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

#include "dynamicVertices.h"
#include "config_char.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "ioPtaDatagramLinMath.h"

TypeHandle DynamicVertices::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DynamicVertices::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DynamicVertices::
DynamicVertices() {
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicVertices::Copy Constructor
//       Access: Public
//  Description: Makes a copy of the DynamicVertices object by
//               reference: the new copy shares the same pointers as
//               the original.
////////////////////////////////////////////////////////////////////
DynamicVertices::
DynamicVertices(const DynamicVertices &copy) :
  _coords(copy._coords),
  _norms(copy._norms),
  _colors(copy._colors),
  _texcoords(copy._texcoords)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicVertices::named deep_copy Constructor
//       Access: Public
//  Description: Makes a complete copy of the indicated
//               DynamicVertices object, including copying all of its
//               arrays.
////////////////////////////////////////////////////////////////////
DynamicVertices DynamicVertices::
deep_copy(const DynamicVertices &copy) {
  DynamicVertices dv;
  if (!copy._coords.empty()) {
    dv._coords = PTA_Vertexf::empty_array(0);
    dv._coords.v() = copy._coords.v();
  }
  if (!copy._norms.empty()) {
    dv._norms = PTA_Normalf::empty_array(0);
    dv._norms.v() = copy._norms.v();
  }
  if (!copy._colors.empty()) {
    dv._colors = PTA_Colorf::empty_array(0);
    dv._colors.v() = copy._colors.v();
  }
  if (!copy._texcoords.empty()) {
    dv._texcoords = PTA_TexCoordf::empty_array(0);
    dv._texcoords.v() = copy._texcoords.v();
  }
  return dv;
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicVertices::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void DynamicVertices::
write_datagram(BamWriter *manager, Datagram &me)
{
  WRITE_PTA(manager, me, IPD_Vertexf::write_datagram, _coords)
  WRITE_PTA(manager, me, IPD_Normalf::write_datagram, _norms)
  WRITE_PTA(manager, me, IPD_Colorf::write_datagram, _colors)
  WRITE_PTA(manager, me, IPD_TexCoordf::write_datagram, _texcoords)
}

////////////////////////////////////////////////////////////////////
//     Function: DynamicVertices::fillin
//       Access: Public
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void DynamicVertices::
fillin(DatagramIterator& scan, BamReader* manager)
{
  READ_PTA(manager, scan, IPD_Vertexf::read_datagram, _coords)
  READ_PTA(manager, scan, IPD_Normalf::read_datagram, _norms)
  READ_PTA(manager, scan, IPD_Colorf::read_datagram, _colors)
  READ_PTA(manager, scan, IPD_TexCoordf::read_datagram, _texcoords)
}
