// Filename: geomLine.cxx
// Created by:  charles (13Jul00)
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

#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#include <ioPtaDatagramShort.h>
#include <ioPtaDatagramInt.h>
#include <ioPtaDatagramLinMath.h>
#include <graphicsStateGuardianBase.h>

#include "geomLine.h"

TypeHandle GeomLine::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomLine::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomLine::
make_copy() const {
  return new GeomLine(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLine::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomLine::
draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_line(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLine::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void GeomLine::
write_datagram(BamWriter *manager, Datagram &me)
{
  Geom::write_datagram(manager, me);
  // Changed from uint32 to float32 on 1/16/02; didn't bother to
  // update the bam version on the assumption that no actual bam files
  // contain GeomLine objects.
  me.add_float32(_width);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLine::make_GeomLine
//       Access: Protected
//  Description: Factory method to generate a GeomLine object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomLine::
make_GeomLine(const FactoryParams &params) {
  GeomLine *me = new GeomLine;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLine::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void GeomLine::
fillin(DatagramIterator& scan, BamReader* manager) {
  Geom::fillin(scan, manager);
  _width = scan.get_float32();
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLine::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomLine object
////////////////////////////////////////////////////////////////////
void GeomLine::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomLine);
}
