// Filename: geomPolygon.cxx
// Created by:  charles (13Jul00)
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

#include "geomPolygon.h"

TypeHandle GeomPolygon::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomPolygon::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomPolygon::
make_copy() const {
  return new GeomPolygon(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPolygon::print_draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomPolygon::
print_draw_immediate(void) const {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPolygon::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomPolygon::
draw_immediate(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_polygon(this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPolygon::make_GeomPolygon
//       Access: Protected
//  Description: Factory method to generate a GeomPolygon object
////////////////////////////////////////////////////////////////////
TypedWriteable* GeomPolygon::
make_GeomPolygon(const FactoryParams &params) {
  GeomPolygon *me = new GeomPolygon;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomPolygon::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomPolygon object
////////////////////////////////////////////////////////////////////
void GeomPolygon::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomPolygon);
}

