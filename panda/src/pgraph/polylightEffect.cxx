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
#include "pmap.h"
#include "colorScaleAttrib.h"
#include "cullTraverserData.h"

#include <math.h>

TypeHandle PolylightEffect::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::make
//       Access: Published, Static
//  Description: Constructs a new PolylightEffect object.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
make() {
  PolylightEffect *effect = new PolylightEffect;
  effect->_contribution_type = CT_proximal;
  effect->_weight = 1.0; // 0.9; // Asad: I don't think we should monkey with the weight.
  effect->_effect_center = LPoint3f(0.0,0.0,0.0);
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::make
//       Access: Published, Static
//  Description: Constructs a new PolylightEffect object.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
make(float weight, ContribType contrib, LPoint3f effect_center) {
  PolylightEffect *effect = new PolylightEffect;
  effect->_contribution_type = contrib;
  effect->_weight = weight;
  effect->_effect_center = effect_center;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::make
//       Access: Published, Static
//  Description: Constructs a new PolylightEffect object.
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
make(float weight, ContribType contrib, LPoint3f effect_center,
     const LightGroup &lights) {
  PolylightEffect *effect = new PolylightEffect;
  effect->_contribution_type = contrib;
  effect->_weight = weight;
  effect->_effect_center = effect_center;
  effect->_lightgroup = lights;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this effect during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool PolylightEffect::
has_cull_callback() const {
  return !_lightgroup.empty();
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.  This may include additional manipulation
//               of render state or additional visible/invisible
//               decisions, or any other arbitrary operation.
//
//               At the time this function is called, the current
//               node's transform and state have not yet been applied
//               to the net_transform and net_state.  This callback
//               may modify the node_transform and node_state to apply
//               an effective change to the render state at this
//               level.
////////////////////////////////////////////////////////////////////
void PolylightEffect::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  CPT(RenderAttrib) poly_light_attrib = do_poly_light(trav->get_scene()->get_scene_root(), &data, node_transform); 
  CPT(RenderState) poly_light_state = RenderState::make(poly_light_attrib);
  node_state = node_state->compose(poly_light_state); 
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::do_poly_light
//       Access: Public
//  Description: Gets the node's position and based on distance from 
//               lights in the lightgroup calculates the color to be 
//               modulated in. If the avatar is in the range of 
//               multiple lights, then determine, which light it is
//               closer to, and get the weight of the scene_color
//               in respect to that light's proximity.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) PolylightEffect::
do_poly_light(const NodePath & root, const CullTraverserData *data, const TransformState *node_transform) const {
  //static bool was_under_polylight = false;
  float dist; // To calculate the distance of each light from the node
  float r,g,b; // To hold the color calculation
  float min_dist; // hold the dist from light that avatar is closer to
  int num_lights = 0; // Keep track of number of lights for division
  float light_scale; // Variable to calculate attenuation 
  float weight_scale; // Variable to compensate snap of color when you walk inside the light volume
  float Rcollect, Gcollect, Bcollect;

  // Initialize Color variables
  r = g = b = 1.0;
  Rcollect = Gcollect = Bcollect = 0.0;

  // get the avatar's base color scale
  Colorf scene_color = root.get_color_scale();
  if (polylight_info) {
    pgraph_cat.info() << "scene color scale = " << scene_color << endl;
  }
  min_dist = 100000.0;
  // Cycle through all the lights in this effect's lightgroup
  LightGroup::const_iterator light_iter; 
  for (light_iter = _lightgroup.begin(); light_iter != _lightgroup.end(); light_iter++){
    const PolylightNode *light = DCAST(PolylightNode, (*light_iter).node()); 

    // light holds the current PolylightNode
    if (light->is_enabled()) { // if enabled get all the properties
      float light_radius = light->get_radius();
      // Calculate the distance of the node from the light
      //dist = light_iter->second->get_distance(data->_node_path.get_node_path());
      const NodePath lightnp = *light_iter;
      LPoint3f point = data->_node_path.get_node_path().get_relative_point(lightnp,
        light->get_pos());

      if (_effect_center[2]) {
        dist = (point - _effect_center).length(); // this counts height difference
      } else {
        // get distance as if the light is at the same height of player
        LVector2f xz(point[0], point[1]);
        dist = xz.length(); // this does not count height difference
      }

      if (dist <= light_radius) { // If node is in range of this light
        if (polylight_info) {
          pgraph_cat.debug() << "light's position = " << light->get_pos() << endl;
          pgraph_cat.debug() << "relative position = " << point << endl;
          pgraph_cat.debug() << "effect center = " << _effect_center << endl;
          //pgraph_cat.info() << "close to this light = " << light->get_name() << endl;
          pgraph_cat.info() << "dist = " << dist << ";radius = " << light_radius << endl;
        }

        PolylightNode::Attenuation_Type light_attenuation = light->get_attenuation();
        Colorf light_color;
        if (light->is_flickering()) { // If flickering, modify color
          light_color = light->flicker();
        } else {
          light_color = light->get_color();
          //light_color = light->get_color_scenegraph();
        }

        float ratio = dist/light_radius;
        if (light_attenuation == PolylightNode::ALINEAR) {
          light_scale = 1.0 - ratio;
        } else if (light_attenuation == PolylightNode::AQUADRATIC) {
          /*
          float fd; // Variable for quadratic attenuation
          float light_a0 = light->get_a0();
          float light_a1 = light->get_a1();
          float light_a2 = light->get_a2();
          if (light_a0 == 0 && light_a1 == 0 && light_a2 == 0) { // To prevent division by zero
            light_a0 = 1.0;
          }
          fd = 1.0 / (light_a0 + light_a1 * dist + light_a2 * dist * dist);
          if (fd < 1.0) {
            light_scale = fd;
          } else {
            light_scale = 1.0;
          }
          */
          //light_scale = 1.0 - ratio*ratio; // graph of 1-x^2
          ratio = 1 - ratio;
          if (ratio <= 0.8)
            light_scale = (ratio*ratio)*(3-2*ratio); //graph of x^2(3-x)
          else
            light_scale = 1.0;
        } else {
          light_scale = 1.0;
        }

        if (min_dist > dist) {
          min_dist = dist;
          // Keep accumulating each lights contribution... 
          // we have to prevent color snap, so factor in the weight.
          // weight becomes negligent as you are closer to the light
          // and opposite otherwise
          weight_scale = _weight * (1.0 - light_scale);
        }

        if (polylight_info) {
          pgraph_cat.info() << "weight_scale = " << weight_scale
                             << "; light_scale " << light_scale << endl;
        }

        Rcollect += light_color[0] * light_scale;
        Gcollect += light_color[1] * light_scale;
        Bcollect += light_color[2] * light_scale;
        /*
        Rcollect += light_color[0] * light_scale + scene_color[0] * weight_scale;
        Gcollect += light_color[1] * light_scale + scene_color[1] * weight_scale;
        Bcollect += light_color[2] * light_scale + scene_color[2] * weight_scale;
        */
        /*
        Rcollect += light_color[0] * light_scale + weight_scale;
        Gcollect += light_color[1] * light_scale + weight_scale;
        Bcollect += light_color[2] * light_scale + weight_scale;
        */


        num_lights++;
      } // if dist< radius
    } // if light is enabled
  } // for all lights

  if ( _contribution_type == CT_all) {
    // Sometimes to prevent snapping of color at light volume boundaries
    // just divide total contribution by all the lights in the effect
    // whether or not they contribute color
    num_lights = _lightgroup.size();
  }

  if (num_lights) {
    //was_under_polylight = true;
    //data->_node_path.get_node_path().set_color_scale_off();
    if (polylight_info)
      pgraph_cat.info() << "num lights = " << num_lights << endl;

    // divide by number of lights to get average.
    r = Rcollect;// / num_lights;
    g = Gcollect;// / num_lights;
    b = Bcollect;// / num_lights;

    if (polylight_info)
      pgraph_cat.info() << "avg: r=" << r << "; g=" << g << "; b=" << b << endl;

    // Now add the scene_color multiplied by weight_scale
    r += scene_color[0] * weight_scale;
    g += scene_color[1] * weight_scale;
    b += scene_color[2] * weight_scale;
    if (polylight_info)
      pgraph_cat.info() << "weighed: r=" << r << "; g=" << g << "; b=" << b << endl;

    /*
    // normalize the color
    LVector3f color_vector(r, g, b);
    color_vector.normalize();
    r = color_vector[0];
    g = color_vector[1];
    b = color_vector[2];

    if (polylight_info)
      pgraph_cat.info() << "unit: r=" << r << "; g=" << g << "; b=" << b << endl;
    */

    // cap it
    r = (r > 1.0)? 1.0 : r;
    g = (g > 1.0)? 1.0 : g;
    b = (b > 1.0)? 1.0 : b;

    if (polylight_info)
      pgraph_cat.info() << "capped: r=" << r << "; g=" << g << "; b=" << b << endl;

    // since this rgb will be scaled by scene_color by day night
    // cycle, lets undo that effect by dividing this rgb by the
    // scene_color. That way, the final render will contain this rgb
    if (scene_color[0] >= 0.01)
      r /= scene_color[0];
    if (scene_color[1] >= 0.01)
      g /= scene_color[1];
    if (scene_color[2] >= 0.01)
      b /= scene_color[2];

    if (polylight_info)
      pgraph_cat.info() << "final: r=" << r << "; g=" << g << "; b=" << b << endl;

  }
  /*
  else {
    if (was_under_polylight) {
      // under no polylight influence...so clear the color scale
      //data->_node_path.get_node_path().clear_color_scale();
      //data->_node_path.get_node_path().set_color_scale(scene_color);
      was_under_polylight = false;
    }
  }
  */

  return ColorScaleAttrib::make(LVecBase4f(r, g, b, 1.0));
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::output
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void PolylightEffect::
output(ostream &out) const {
  out << get_type() << ":";
    
  LightGroup::const_iterator li;
  for (li = _lightgroup.begin(); li != _lightgroup.end(); ++li) {
    NodePath light = (*li);
    out << " " << light;
  }
  out << " weight " << _weight << " contrib " << _contribution_type
      << " center " << _effect_center;
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



////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::add_light
//       Access: Published
//  Description: Add a PolylightNode object to this effect and return
//               a new effect
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
add_light(const NodePath &newlight) const {
  PolylightEffect *effect = new PolylightEffect(*this);
  effect->_lightgroup.push_back(newlight);
  return return_new(effect);
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::remove_light
//       Access: Published
//  Description: Remove a light from this effect. Return the new updated
//               effect
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
remove_light(const NodePath &newlight) const {
  PolylightEffect *effect = new PolylightEffect(*this);
  LightGroup::iterator light_iter;
  light_iter = find(effect->_lightgroup.begin(),effect->_lightgroup.end(), newlight); 
  if (light_iter == effect->_lightgroup.end()) {
    pgraph_cat.debug()
      << "Attempt to remove Polylight " << newlight << "; not found.\n";
  } else {
    // Remove light
    effect->_lightgroup.erase(light_iter);
  }
  return return_new(effect);
 
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::set_weight
//       Access: Published
//  Description: Set weight and return a new effect... the reason
//               this couldnt be done through make was because
//               that would return a new effect without the 
//               lightgroup which is static and cant be accessed
//               Here, we just pass that to the make
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
set_weight(float w) const {
  PolylightEffect *effect = new PolylightEffect(*this);
  effect->_weight = w;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::set_contrib
//       Access: Published
//  Description: Set Contrib Type and return a new effect... the reason
//               this couldnt be done through make was because
//               that would return a new effect without the 
//               lightgroup which is static and cant be accessed
//               Here, we just pass that to the make
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
set_contrib(ContribType ct) const {
  PolylightEffect *effect = new PolylightEffect(*this);
  effect->_contribution_type = ct;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::set_effect_center
//       Access: Published
//  Description: Set weight and return a new effect... the reason
//               this couldnt be done through make was because
//               that would return a new effect without the 
//               lightgroup which is static and cant be accessed
//               Here, we just pass that to the make
////////////////////////////////////////////////////////////////////
CPT(RenderEffect) PolylightEffect::
set_effect_center(LPoint3f ec) const{
  PolylightEffect *effect = new PolylightEffect(*this);
  effect->_effect_center = ec;
  return return_new(effect);
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::has_light
//       Access: Published
//  Description: Returns true if the indicated light is listed in the
//               PolylightEffect, false otherwise.
////////////////////////////////////////////////////////////////////
bool PolylightEffect::
has_light(const NodePath &light) const {
  LightGroup::const_iterator li;
  li = find(_lightgroup.begin(), _lightgroup.end(), light); 
  return (li != _lightgroup.end());
}

ostream &
operator << (ostream &out, PolylightEffect::ContribType ct) {
  switch (ct) {
  case PolylightEffect::CT_proximal:
    return out << "proximal";

  case PolylightEffect::CT_all:
    return out << "all";
  }

  return out << "**Invalid ContribType(" << (int)ct << ")**";
}
