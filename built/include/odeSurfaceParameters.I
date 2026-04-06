/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeSurfaceParameters.I
 * @author joswilso
 * @date 2006-12-27
 */

INLINE void OdeSurfaceParameters::
set_mode(int mode) {
  _surface_parameters.mode = mode;
}

INLINE void OdeSurfaceParameters::
set_mu(dReal mu) {
  _surface_parameters.mu = mu;
}

INLINE void OdeSurfaceParameters::
set_mu2(dReal mu2) {
  _surface_parameters.mu2 = mu2;
}

INLINE void OdeSurfaceParameters::
set_bounce(dReal bounce) {
  _surface_parameters.bounce = bounce;
}

INLINE void OdeSurfaceParameters::
set_bounce_vel(dReal bounce_vel) {
  _surface_parameters.bounce_vel = bounce_vel;
}

INLINE void OdeSurfaceParameters::
set_soft_erp(dReal soft_erp) {
  _surface_parameters.soft_erp = soft_erp;
}

INLINE void OdeSurfaceParameters::
set_soft_cfm(dReal soft_cfm) {
  _surface_parameters.soft_cfm = soft_cfm;
}

INLINE void OdeSurfaceParameters::
set_motion1(dReal motion) {
  _surface_parameters.motion1 = motion;
}

INLINE void OdeSurfaceParameters::
set_motion2(dReal motion) {
  _surface_parameters.motion2 = motion;
}

INLINE void OdeSurfaceParameters::
set_slip1(dReal slip) {
  _surface_parameters.slip1 = slip;
}

INLINE void OdeSurfaceParameters::
set_slip2(dReal slip) {
  _surface_parameters.slip2 = slip;
}


INLINE int OdeSurfaceParameters::
get_mode() const {
  return _surface_parameters.mode;
}

INLINE dReal OdeSurfaceParameters::
get_mu() const {
  return _surface_parameters.mu;
}

INLINE dReal OdeSurfaceParameters::
get_mu2() const {
  return _surface_parameters.mu2;
}

INLINE dReal OdeSurfaceParameters::
get_bounce() const {
  return _surface_parameters.bounce;
}

INLINE dReal OdeSurfaceParameters::
get_bounce_vel() const {
  return _surface_parameters.bounce_vel;
}

INLINE dReal OdeSurfaceParameters::
get_soft_erp() const {
  return _surface_parameters.soft_erp;
}

INLINE dReal OdeSurfaceParameters::
get_soft_cfm() const {
  return _surface_parameters.soft_cfm;
}

INLINE dReal OdeSurfaceParameters::
get_motion1() const {
  return _surface_parameters.motion1;
}

INLINE dReal OdeSurfaceParameters::
get_motion2() const {
  return _surface_parameters.motion2;
}

INLINE dReal OdeSurfaceParameters::
get_slip1() const {
  return _surface_parameters.slip1;
}

INLINE dReal OdeSurfaceParameters::
get_slip2() const {
  return _surface_parameters.slip2;
}
