// Filename: geomSphere.cxx
// Created by:  drose (01Oct99)
// 
////////////////////////////////////////////////////////////////////

#include "geomSphere.h"
#include <graphicsStateGuardianBase.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

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
draw_immediate(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_sphere(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSphere::make_GeomSphere
//       Access: Protected
//  Description: Factory method to generate a GeomSphere object
////////////////////////////////////////////////////////////////////
TypedWriteable* GeomSphere::
make_GeomSphere(const FactoryParams &params)
{
  GeomSphere *me = new GeomSphere;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
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




