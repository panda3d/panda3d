// Filename: geomSprite.cxx
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
#include "ioPtaDatagramFloat.h"
#include "ioPtaDatagramLinMath.h"
#include "graphicsStateGuardianBase.h"

#include "geomSprite.h"

TypeHandle GeomSprite::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite
//       Access: public
//  Description: constructor
////////////////////////////////////////////////////////////////////
GeomSprite::
GeomSprite(Texture *tex, bool alpha_disable) :
  _texture(tex), _alpha_disable(alpha_disable) 
{
  _ll_uv.set(0.0f, 0.0f);
  _ur_uv.set(1.0f, 1.0f);
  _x_texel_ratio.clear();
  _y_texel_ratio.clear();

  _theta_bind_type = G_OFF;
  // note that the other bind types are intentionally left
  // uninitialized; the arrays themselves can not be set without a
  // bind type.
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Geom that is a shallow copy
//               of this one.  It will be a different Geom pointer,
//               but its internal data may or may not be shared with
//               that of the original Geom.
////////////////////////////////////////////////////////////////////
Geom *GeomSprite::
make_copy() const {
  return new GeomSprite(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::print_draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomSprite::
print_draw_immediate(void) const {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::draw_immediate
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void GeomSprite::
draw_immediate(GraphicsStateGuardianBase *gsg, GeomContext *gc) {
  gsg->draw_sprite(this, gc);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::is_dynamic
//       Access: Public, Virtual
//  Description: Returns true if the Geom has any dynamic properties
//               that are expected to change from one frame to the
//               next, or false if the Geom is largely static.  For
//               now, this is the same thing as asking whether its
//               vertices are indexed.
////////////////////////////////////////////////////////////////////
bool GeomSprite::
is_dynamic() const {
  // Sprites are always dynamic.
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void GeomSprite::
write_datagram(BamWriter *manager, Datagram &me) {
  Geom::write_datagram(manager, me);
  WRITE_PTA(manager, me, IPD_float::write_datagram, _x_texel_ratio);
  WRITE_PTA(manager, me, IPD_float::write_datagram, _y_texel_ratio);
  me.add_uint8(_x_bind_type);
  me.add_uint8(_y_bind_type);
  me.add_uint8(_alpha_disable);
  manager->write_pointer(me, _texture);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::make_GeomSprite
//       Access: Protected
//  Description: Factory method to generate a GeomSprite object
////////////////////////////////////////////////////////////////////
TypedWritable* GeomSprite::
make_GeomSprite(const FactoryParams &params) {
  GeomSprite *me = new GeomSprite;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  me->make_dirty();
  me->config();
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void GeomSprite::
fillin(DatagramIterator& scan, BamReader* manager) {
  Geom::fillin(scan, manager);
  READ_PTA(manager, scan, IPD_float::read_datagram, _x_texel_ratio);
  READ_PTA(manager, scan, IPD_float::read_datagram, _y_texel_ratio);
  _x_bind_type = (GeomBindType) scan.get_uint8();
  _y_bind_type = (GeomBindType) scan.get_uint8();
  _alpha_disable = (scan.get_uint8() !=0);
  manager->read_pointer(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a GeomSprite object
////////////////////////////////////////////////////////////////////
void GeomSprite::
register_with_read_factory(void) {
  BamReader::get_factory()->register_factory(get_class_type(), make_GeomSprite);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomSprite::complete_pointers
//       Access: Public, Static
//  Description:
////////////////////////////////////////////////////////////////////
int GeomSprite::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = Geom::complete_pointers(p_list, manager);
  _texture = DCAST(Texture, p_list[index]);

  return index + 1;
}
