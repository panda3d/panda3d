// Filename: odeContact.I
// Created by:  joswilso (27Dec06)
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

INLINE OdeSurfaceParameters OdeContact::
get_surface() const {
  return OdeSurfaceParameters(_contact.surface);
}

INLINE OdeContactGeom OdeContact::
get_geom() {
  return OdeContactGeom(_contact.geom);
}

INLINE LVecBase3f OdeContact::
get_fdir1() const {
  return LVecBase3f(_contact.fdir1[0],
                    _contact.fdir1[1],
                    _contact.fdir1[2]);
}


INLINE void OdeContact::
set_surface(const OdeSurfaceParameters &surface_parameters) {
  _contact.surface = *(surface_parameters.get_surface_parameters_ptr());
}

INLINE void OdeContact::
set_geom(const OdeContactGeom &contact_geom) {
  _contact.geom = *(contact_geom.get_contact_geom_ptr());
}

INLINE void OdeContact::
set_fdir1(const LVecBase3f &fdir1) {
  _contact.fdir1[0] = fdir1[0];
  _contact.fdir1[1] = fdir1[1];
  _contact.fdir1[2] = fdir1[2];
}


