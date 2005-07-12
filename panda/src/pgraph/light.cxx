// Filename: light.cxx
// Created by:  mike (09Jan97)
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
get_vector_to_light(LVector3f &, const LPoint3f &, const LMatrix4f &) {
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
  CDReader cdata(_cycler);
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
