/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeWorld.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "config_ode.h"
#include "odeWorld.h"
#include "odeBody.h"

TypeHandle OdeWorld::_type_handle;

OdeWorld::
OdeWorld() :
  _id(dWorldCreate()) {
  if (odeworld_cat.is_debug()) {
    odeworld_cat.debug() << get_type() << "(" << _id << ")" << "\n";
  }
  _num_surfaces = 0;

}

OdeWorld::
OdeWorld(const OdeWorld &copy) :
  _id(copy._id) {
  _num_surfaces = 0;

}

OdeWorld::
~OdeWorld() {
  if (odeworld_cat.is_debug()) {
    odeworld_cat.debug() << "~" << get_type() << "(" << _id << ")" << "\n";
  }
}

void OdeWorld::
destroy() {
  if(_num_surfaces > 0) {
    delete _surface_table;
  }
  nassertv(_id);
  dWorldDestroy(_id);
}

/*
void OdeWorld::
assign_surface_body(OdeBody& body, int surface) {
  // odeworld_cat.debug() << "assign_surface_body body.Id =" << body.get_id()
  // << " surface=" << surface << "\n";
  _body_dampen_map[body.get_id()].surfaceType = surface;
  _body_dampen_map[body.get_id()].dampen = 0.0f;
}
*/

void OdeWorld::
add_body_dampening(OdeBody& body, int surface) {
  _body_dampen_map[body.get_id()].dampen = 0.0f;
}


void OdeWorld::
init_surface_table(uint8_t num_surfaces) {
  _surface_table = new sSurfaceParams[num_surfaces * num_surfaces];
  // _dampen_table = new sSurfaceParams[num_surfaces * num_surfaces];
  _num_surfaces = num_surfaces;
}

void OdeWorld::
set_surface(int pos1, int pos2, sSurfaceParams& entry) {
  if (odeworld_cat.is_debug()) {
    odeworld_cat.debug() << " pos1 " << pos1 << " pos2 " << pos2 << " num surfaces " << (int)_num_surfaces << " endline\n";
  }
  if((_num_surfaces <= pos1) || (_num_surfaces <= pos2)) {
    odeworld_cat.error() << "surface position exceeds size of surface table, set num_surface in initSurfaceTable higher." << "\n";
    return;
  }
  int true_pos = (pos1 * _num_surfaces) + pos2;
  _surface_table[true_pos].colparams.mode = entry.colparams.mode;
  _surface_table[true_pos].colparams.mu = entry.colparams.mu;
  _surface_table[true_pos].colparams.mu2 = entry.colparams.mu2;
  _surface_table[true_pos].colparams.bounce = entry.colparams.bounce;
  _surface_table[true_pos].colparams.bounce_vel = entry.colparams.bounce_vel;
  _surface_table[true_pos].colparams.soft_cfm = entry.colparams.soft_cfm;
  _surface_table[true_pos].colparams.motion1 = entry.colparams.motion1;
  _surface_table[true_pos].colparams.motion2 = entry.colparams.motion2;
  _surface_table[true_pos].colparams.slip1 = entry.colparams.slip1;
  _surface_table[true_pos].colparams.slip2 = entry.colparams.slip2;
  _surface_table[true_pos].dampen = entry.dampen;
}


sSurfaceParams& OdeWorld::
get_surface(uint8_t surface1, uint8_t surface2) {
  int true_pos = 0;
  if(surface1 >= surface2) {
    true_pos = (surface1 * _num_surfaces) + surface2;
  } else {
    true_pos = (surface2 * _num_surfaces) + surface1;
  }
  if((_num_surfaces <= surface1) || (_num_surfaces <= surface2)) {
    odeworld_cat.error() << "surface position exceeds size of surface table, set num_surface in initSurfaceTable higher." << "\n";
    // nassertr_always((_num_surfaces > surface1 && _num_surfaces > surface2),
    // _surface_table[true_pos]);
  }
  return _surface_table[true_pos];
}

void OdeWorld::
set_surface_entry(uint8_t pos1, uint8_t pos2,
                  dReal mu,
                  dReal bounce,
                  dReal bounce_vel,
                  dReal soft_erp,
                  dReal soft_cfm,
                  dReal slip,
                  dReal dampen) {
  // todo: add mode
  sSurfaceParams new_params;
  int someMode = 0;
  if (bounce > 0.0001) {
    someMode |= dContactBounce;
  }
  if (soft_erp > 0.0001) {
    someMode |= dContactSoftERP;
  }
  if (soft_cfm > 0.0001) {
    someMode |= dContactSoftCFM;
  }
  if (slip > 0.0001) {
    someMode = someMode | dContactSlip1 | dContactSlip2;
  }
  new_params.colparams.mode = dContactBounce | dContactSoftCFM | dContactApprox1;// | dContactSoftERP;
  new_params.colparams.mu = mu;
  new_params.colparams.mu2 = mu;
  new_params.colparams.bounce = bounce;
  new_params.colparams.bounce_vel = bounce_vel;
  new_params.colparams.soft_erp = soft_erp;
  new_params.colparams.soft_cfm = soft_cfm;
  new_params.colparams.slip1 = slip;
  new_params.colparams.slip2 = slip;
  new_params.colparams.motion1 = 0.0;
  new_params.colparams.motion2 = 0.0;
  new_params.dampen = dampen;
  // todo: a bit of wasted space here
  set_surface(pos1, pos2, new_params);

  if(pos1 >= pos2) {
    set_surface(pos1, pos2, new_params);
  } else {
    set_surface(pos2, pos1, new_params);
  }
}



void OdeWorld::
set_dampen_on_bodies(dBodyID id1, dBodyID id2,dReal damp) {
  if(_body_dampen_map[id1].dampen < damp) {
    _body_dampen_map[id1].dampen = damp;
  }
  if(_body_dampen_map[id2].dampen < damp) {
    _body_dampen_map[id2].dampen = damp;
  }
}

float OdeWorld::
apply_dampening(float dt, OdeBody& body) {
  dBodyID bodyId = body.get_id();
  dReal damp = _body_dampen_map[bodyId].dampen;
  float dampening = 1.00 - (damp * dt);
  body.set_angular_vel(body.get_angular_vel() * dampening);
  body.set_linear_vel(body.get_linear_vel() * dampening);
  _body_dampen_map[bodyId].dampen = 0.0;
  return dampening;
}

OdeWorld::
operator bool () const {
  return (_id != nullptr);
}
