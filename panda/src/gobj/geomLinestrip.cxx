// Filename: geomLinestrip.cxx
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
draw_immediate(GraphicsStateGuardianBase *gsg) const {
  gsg->draw_linestrip(this);
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
  me.add_uint32(_width);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomLinestrip::make_GeomLinestrip
//       Access: Protected
//  Description: Factory method to generate a GeomLinestrip object
////////////////////////////////////////////////////////////////////
TypedWriteable* GeomLinestrip::
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
  _width = scan.get_uint32();
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
