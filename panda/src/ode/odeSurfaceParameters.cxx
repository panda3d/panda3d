// Filename: odeSurfaceParameters.cxx
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

#include "config_ode.h"
#include "odeSurfaceParameters.h"

TypeHandle OdeSurfaceParameters::_type_handle;

OdeSurfaceParameters::
OdeSurfaceParameters(int mode, dReal mu) : 
  _surface_parameters() {
  if (mu < 0) {
    mu = 0;
  } else if (mu > dInfinity) {
    mu = dInfinity;
  }
  _surface_parameters.mode = mode;
  _surface_parameters.mu = mu;

  _surface_parameters.mu2 = 0;
  _surface_parameters.bounce = 0;
  _surface_parameters.bounce = 0;
  _surface_parameters.soft_erp = 0;
  _surface_parameters.soft_cfm = 0;
  _surface_parameters.motion1 = 0;
  _surface_parameters.motion2 = 0;
  _surface_parameters.slip1 = 0;
  _surface_parameters.slip2 = 0;
}

OdeSurfaceParameters::
OdeSurfaceParameters(const dSurfaceParameters &surface_parameters) : 
  _surface_parameters() {
  _surface_parameters.mode = surface_parameters.mode;
  _surface_parameters.mu = surface_parameters.mu;
  _surface_parameters.mu2 = surface_parameters.mu2;
  _surface_parameters.bounce = surface_parameters.bounce;
  _surface_parameters.bounce = surface_parameters.bounce_vel;
  _surface_parameters.soft_erp = surface_parameters.soft_erp;
  _surface_parameters.soft_cfm = surface_parameters.soft_cfm;
  _surface_parameters.motion1 = surface_parameters.motion1;
  _surface_parameters.motion2 = surface_parameters.motion2;
  _surface_parameters.slip1 = surface_parameters.slip1;
  _surface_parameters.slip2 = surface_parameters.slip2;
}

OdeSurfaceParameters::
~OdeSurfaceParameters() {
}

const dSurfaceParameters *OdeSurfaceParameters::
get_surface_parameters_ptr() const {
  return &_surface_parameters;
}

void OdeSurfaceParameters::
operator = (const OdeSurfaceParameters &copy) {
  _surface_parameters.mode = copy._surface_parameters.mode;
  _surface_parameters.mu = copy._surface_parameters.mu;
  _surface_parameters.mu2 = copy._surface_parameters.mu2;
  _surface_parameters.bounce = copy._surface_parameters.bounce;
  _surface_parameters.bounce = copy._surface_parameters.bounce_vel;
  _surface_parameters.soft_erp = copy._surface_parameters.soft_erp;
  _surface_parameters.soft_cfm = copy._surface_parameters.soft_cfm;
  _surface_parameters.motion1 = copy._surface_parameters.motion1;
  _surface_parameters.motion2 = copy._surface_parameters.motion2;
  _surface_parameters.slip1 = copy._surface_parameters.slip1;
  _surface_parameters.slip2 = copy._surface_parameters.slip2;
}
