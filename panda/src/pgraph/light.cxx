// Filename: light.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "light.h"
#include "bamWriter.h"
#include "bamReader.h"
#include "datagram.h"
#include "datagramIterator.h"

UpdateSeq Light::_sort_seq;

TypeHandle Light::_type_handle;


////////////////////////////////////////////////////////////////////
//     Function: Light::CData::make_copy
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
CycleData *Light::CData::
make_copy() const {
  return new CData(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: Light::CData::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Light::CData::
write_datagram(BamWriter *, Datagram &dg) const {
  _color.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: Light::CData::fillin
//       Access: Public, Virtual
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Light.
////////////////////////////////////////////////////////////////////
void Light::CData::
fillin(DatagramIterator &scan, BamReader *) {
  _color.read_datagram(scan);
}

////////////////////////////////////////////////////////////////////
//     Function: Light::Destructor
//       Access: Published, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
Light::
~Light() {
}

////////////////////////////////////////////////////////////////////
//     Function: Light::is_ambient_light
//       Access: Published, Virtual
//  Description: Returns true if this is an AmbientLight, false if it
//               is some other kind of light.
////////////////////////////////////////////////////////////////////
bool Light::
is_ambient_light() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_exponent
//       Access: Public, Virtual
//  Description: For spotlights, returns the exponent that controls
//               the amount of light falloff from the center of the
//               spotlight.  For other kinds of lights, returns 0.
////////////////////////////////////////////////////////////////////
PN_stdfloat Light::
get_exponent() const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_specular_color
//       Access: Public, Virtual
//  Description: Returns the color of specular highlights generated
//               by the light.  This value is meaningless for ambient
//               lights.
////////////////////////////////////////////////////////////////////
const LColor &Light::
get_specular_color() const {
  static const LColor white(1, 1, 1, 1);
  return white;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_attenuation
//       Access: Public, Virtual
//  Description: Returns the terms of the attenuation equation for the
//               light.  These are, in order, the constant, linear,
//               and quadratic terms based on the distance from the
//               point to the vertex.
////////////////////////////////////////////////////////////////////
const LVecBase3 &Light::
get_attenuation() const {
  static const LVecBase3 no_atten(1, 0, 0);
  return no_atten;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_vector_to_light
//       Access: Public, Virtual
//  Description: Computes the vector from a particular vertex to this
//               light.  The exact vector depends on the type of light
//               (e.g. point lights return a different result than
//               directional lights).
//
//               The input parameters are the vertex position in
//               question, expressed in object space, and the matrix
//               which converts from light space to object space.  The
//               result is expressed in object space.
//
//               The return value is true if the result is successful,
//               or false if it cannot be computed (e.g. for an
//               ambient light).
////////////////////////////////////////////////////////////////////
bool Light::
get_vector_to_light(LVector3 &, const LPoint3 &, const LMatrix4 &) {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::get_viz
//       Access: Public
//  Description: Returns a GeomNode that may be rendered to visualize
//               the Light.  This is used during the cull traversal to
//               render the Lights that have been made visible.
////////////////////////////////////////////////////////////////////
GeomNode *Light::
get_viz() {
  CDLockedReader cdata(_cycler);
  if (cdata->_viz_geom_stale) {
    CDWriter cdata_w(_cycler, cdata);

    cdata_w->_viz_geom = new GeomNode("viz");
    fill_viz_geom(cdata_w->_viz_geom);
    cdata_w->_viz_geom_stale = false;
  }
  return cdata->_viz_geom;
}

////////////////////////////////////////////////////////////////////
//     Function: Light::fill_viz_geom
//       Access: Protected, Virtual
//  Description: Fills the indicated GeomNode up with Geoms suitable
//               for rendering this light.
////////////////////////////////////////////////////////////////////
void Light::
fill_viz_geom(GeomNode *) {
}

////////////////////////////////////////////////////////////////////
//     Function: Light::write_datagram
//       Access: Protected
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void Light::
write_datagram(BamWriter *manager, Datagram &dg) {
  manager->write_cdata(dg, _cycler);
  dg.add_int32(_priority);
}

////////////////////////////////////////////////////////////////////
//     Function: Light::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new Light.
////////////////////////////////////////////////////////////////////
void Light::
fillin(DatagramIterator &scan, BamReader *manager) {
  manager->read_cdata(scan, _cycler);
  _priority = scan.get_int32();
}
