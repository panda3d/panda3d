// Filename: odeWorld.cxx
// Created by:  joswilso (27Dec06)
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

#include "config_ode.h"
#include "odeWorld.h"

TypeHandle OdeWorld::_type_handle;

OdeWorld::
OdeWorld() : 
  _id(dWorldCreate()) {
  odeworld_cat.debug() << get_type() << "(" << _id << ")" << "\n";
  _num_surfaces = 0;
}

OdeWorld::
OdeWorld(const OdeWorld &copy) :
  _id(copy._id) {
  _num_surfaces = 0;
}

OdeWorld::
~OdeWorld() {
  odeworld_cat.debug() << "~" << get_type() << "(" << _id << ")" << "\n";
}

void OdeWorld::
destroy() {
  if(_num_surfaces > 0)
  {
      delete _surface_table;
  }
  dWorldDestroy(_id);
}

void OdeWorld::
init_surface_table(PN_uint8 num_surfaces)
{
    _surface_table = new dSurfaceParameters[num_surfaces * num_surfaces];
    _num_surfaces = num_surfaces;
}

void OdeWorld::
set_surface(int pos1, int pos2, dSurfaceParameters& entry)
{
    int true_pos = (pos1 * _num_surfaces) + pos2;
    _surface_table[true_pos].mode = entry.mode;
    _surface_table[true_pos].mu = entry.mu;
    _surface_table[true_pos].mu2 = entry.mu2;
    _surface_table[true_pos].bounce = entry.bounce;
    _surface_table[true_pos].bounce_vel = entry.bounce_vel;
    _surface_table[true_pos].soft_cfm = entry.soft_cfm;
    _surface_table[true_pos].motion1 = entry.motion1;
    _surface_table[true_pos].motion2 = entry.motion2;
    _surface_table[true_pos].slip1 = entry.slip1;
    _surface_table[true_pos].slip2 = entry.slip2;
}

dSurfaceParameters& OdeWorld::
get_surface(PN_uint8 surface1, PN_uint8 surface2)
{
    int true_pos = 0;
    if(surface1 >= surface2)
    {
        true_pos = (surface1 * _num_surfaces) + surface2;
    }
    else
    {
        true_pos = (surface2 * _num_surfaces) + surface1;
    }
    return _surface_table[true_pos];
}

void OdeWorld:: 
set_surface_entry(  PN_uint8 pos1, PN_uint8 pos2, 
                    dReal mu,
                    dReal bounce, 
                    dReal bounce_vel, 
                    dReal soft_erp, 
                    dReal soft_cfm,
                    dReal slip)
{
    //todo: add mode
    dSurfaceParameters new_params;
    new_params.mode = 0;
    new_params.mu = mu;
    new_params.mu2 = mu;
    new_params.bounce = bounce;
    new_params.bounce_vel = bounce_vel;
    new_params.soft_erp = soft_erp;
    new_params.soft_cfm = soft_cfm;
    new_params.slip1 = slip;
    new_params.slip2 = slip;
    new_params.motion1 = 0.0;
    new_params.motion2 = 0.0;
    //todo: a bit of wasted space here
    set_surface(pos1, pos2, new_params);
   
    if(pos1 >= pos2)
    {
        set_surface(pos1, pos2, new_params);
    }
    else
    {
        set_surface(pos2, pos1, new_params);
    }
}

void OdeWorld:: 
assign_surface_body(OdeBody& body, int surface)
{
    _body_surface_map[body.get_id()] = surface;
}

int OdeWorld::
get_surface_body(dBodyID id)
{
  BodySurfaceMap::iterator iter = _body_surface_map.find(id);
  if (iter != _body_surface_map.end()) {
    return iter->second;
  }
  return 0;
}
