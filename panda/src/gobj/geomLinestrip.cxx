// Filename: geomLinestrip.cxx
// Created by:  charles (13Jul00)
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

#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "ioPtaDatagramShort.h"
#include "ioPtaDatagramInt.h"
#include "ioPtaDatagramLinMath.h"
#include "graphicsStateGuardianBase.h"

#include "config_gobj.h"
#include "geomLinestrip.h"

TypeHandle GeomLinestrip::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomLinestrip::
make_copy() const {
  return new GeomLinestrip(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomLinestrip::
draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_linestrip(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::explode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Geom *GeomLinestrip::
explode() const {
  gobj_cat.error()
    << "GeomLinestrip::explode() - not implemented yet!!!" << endl;
  return new GeomLinestrip(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void GeomLinestrip::
write_datagram(BamWriter *manager, Datagram &me) {
  Geom::write_datagram(manager, me);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::make_GeomLinestrip
//       Access: Protected
//  Description: Factory method to generate a GeomLinestrip object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomLinestrip::
make_GeomLinestrip(const FactoryParams &params) {
  GeomLinestrip *me = new GeomLinestrip;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void GeomLinestrip::
fillin(DatagramIterator& scan, BamReader* manager) {
  Geom::fillin(scan, manager);

  if (manager->get_file_minor_ver() < 15) {
    // Skip width parameter.
    scan.get_float32();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomLinestrip object
////////////////////////////////////////////////////////////////////
void GeomLinestrip::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomLinestrip);
}
