// Filename: geomSphere.cxx
// Created by:  drose (01Oct99)
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

#include "geomSphere.h"
#include "graphicsStateGuardianBase.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"

TypeHandle GeomSphere::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: GeomSphere::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomSphere::
make_copy() const {
  return new GeomSphere(*this);
}

void GeomSphere::
draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_sphere(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSphere::make_GeomSphere
//       Access: Protected
//  Description: Factory method to generate a GeomSphere object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomSphere::
make_GeomSphere(const FactoryParams &params)
{
  GeomSphere *me = new GeomSphere;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSphere::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomSphere object
////////////////////////////////////////////////////////////////////
void GeomSphere::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomSphere);
}




