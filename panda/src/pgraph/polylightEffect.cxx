// Filename: polylightEffect.cxx
// Created by:  sshodhan (02Jun04)
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

#include "polylightEffect.h"
#include "polylightNode.h"
#include "config_pgraph.h"
#include "nodePath.h"
#include "colorScaleAttrib.h"
#include "pmap.h"
#include <math.h>

TypeHandle PolylightEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::make
//       Access: Published, Static
//  Description: Constructs a new PolylightEffect object.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
make(string contribution_type,float weight) {
  PolylightEffect *effect = new PolylightEffect;
  effect->enable();
  effect->set_contrib(contribution_type);
  effect->set_weight(weight);
  return return_new(effect);
}
////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::do_poly_light
//       Access: Public
//  Description: Gets the node's position and based on distance from 
//  lights in the lightgroup calculates the color to be modulated in
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) PolylightEffect::
do_poly_light(const CullTraverserData *data, const TransformState *node_transform) const {
  float r,g,b; // To hold the color calculation
  float dist; // To calculate the distance of each light from the node
  float light_scale; // Variable to calculate attenuation 
  float fd; // Variable for quadratic attenuation
  float Rcollect=0.0,Gcollect=0.0,Bcollect=0.0; // Color variables
  int num_lights=0; // Keep track of number of lights for division
  r = 0.0;
  g = 0.0;
  b = 0.0;
  LIGHTGROUP::const_iterator light_iter; 
  // Cycle through all the lights in this effect's lightgroup
  for (light_iter = _lightgroup.begin(); light_iter != _lightgroup.end(); light_iter++){
    const PolylightNode *light = DCAST(PolylightNode,light_iter->second->node()); 
    // light holds the current PolylightNode
    if(light->is_enabled()) { // if enabled get all the properties
      float light_radius = light->get_radius();
      string light_attenuation = light->get_attenuation();
      float light_a0 = light->get_a0();
      float light_a1 = light->get_a1();
      float light_a2 = light->get_a2();
      if(light_a0 == 0 && light_a1 == 0 && light_a2 == 0) { // To prevent division by zero
        light_a0 = 1.0;
      }
      Colorf light_color;
      if(light->is_flickering()) { // If flickering, modify color
        light_color = light->flicker();
      }
      else {
        light_color = light->get_color();
      }
    
      // Calculate the distance of the node from the light
      dist = light_iter->second->get_distance(data->_node_path.get_node_path());

      if(dist < light_radius) { // If node is in range of this light
        if(light_attenuation == "linear") {
          light_scale = (light_radius - dist)/light_radius;
        }
        else if(light_attenuation == "quadratic") {
          fd = 1.0 / (light_a0 + light_a1 * dist + light_a2 * dist * dist);
          if(fd<1.0) {
            light_scale=fd;
          }
          else {
            light_scale=1.0;
          }
        }
        // Keep accumulating each lights contribution... we divide by 
        // number of lights later.
        Rcollect += light_color[0] * light_scale;
        Gcollect += light_color[1] * light_scale;
        Bcollect += light_color[2] * light_scale;
        num_lights++;
      } // if dist< radius
    } // if light is enabled
  } // for all lights
  

  if( _contribution_type == "all") {
    // Sometimes to prevent snapping of color at light volume boundaries
    // just divide total contribution by all the lights in the effect
    // whether or not they contribute color
    num_lights = _lightgroup.size();
  }

  if(num_lights == 0) {
    num_lights = 1;
  }
  Rcollect /= num_lights;
  Gcollect /= num_lights;
  Bcollect /= num_lights;

  r = (1.0 - _weight) + Rcollect * _weight;
  g = (1.0 - _weight) + Gcollect * _weight;
  b = (1.0 - _weight) + Bcollect * _weight;
  
  return ColorScaleAttrib::make(LVecBase4f(r,g,b,1.0));
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived PolylightEffect
//               types to return a unique number indicating whether
//               this PolylightEffect is equivalent to the other one.
//
//               This should return 0 if the two PolylightEffect objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two PolylightEffect
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int PolylightEffect::
compare_to_impl(const RenderEffect *other) const {
  const PolylightEffect *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_enabled != ta->_enabled) {
      return _enabled ? 1 : -1;
  }

  if (_contribution_type != ta->_contribution_type) {
    return _contribution_type < ta->_contribution_type ? -1 : 1;
  }
 
  if (_weight != ta->_weight) {
    return _weight < ta->_weight ? -1 :1;
  }

  if (_lightgroup != ta->_lightgroup) {
    return _lightgroup < ta->_lightgroup ? -1 : 1;
  }
 
  return 0;
}

