// Filename: polylightEffect.I
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


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::Constructor
//       Access: Private
//  Description: Use PolylightEffect::make() to construct a new
//               PolylightEffect object.
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::
PolylightEffect() {
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::is_enabled
//       Access: Published
//  Description: Returns state of the effect: enabled/disabled
////////////////////////////////////////////////////////////////////
INLINE bool PolylightEffect::
is_enabled() const {
  return _enabled;
}



////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::enable
//       Access: Published
//  Description: Enables the effect
////////////////////////////////////////////////////////////////////
INLINE void PolylightEffect::
enable() {
  _enabled=true;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::disable
//       Access: Published
//  Description: Disables the effect
////////////////////////////////////////////////////////////////////
INLINE void PolylightEffect::
disable() {
  _enabled=false;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::add_light
//       Access: Published
//  Description: Add a PolylightNode object to this effect and give it a 
//               unique name. Returns true if light added
////////////////////////////////////////////////////////////////////
INLINE bool PolylightEffect::
add_light(const string &lightname, const NodePath &newlight) {
  nassertr(newlight.node()->is_of_type(PolylightNode::get_class_type()) ,false);
  nassertr(newlight.node() != (PolylightNode *)NULL,false);
  
  // Check if light name is unique
  if(_lightgroup.find(lightname) != _lightgroup.end()) {
    cerr << "Light name already exists. Please choose another name\n";
    return false;
  }
    
  // Add the light
  pair<string,NodePath> _light_to_add(lightname, newlight);
  _lightgroup.insert(_light_to_add);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::remove_light
//       Access: Published
//  Description: Remove a light from this effect. Returns true on
//               success.
////////////////////////////////////////////////////////////////////
INLINE bool PolylightEffect::
remove_light(const string &lightname) {

  // Check if light name exists
  if(_lightgroup.find(lightname) == _lightgroup.end()) {
    cerr << "Light name does not exist\n";
    return false;
  }

  // Remove light
  _lightgroup.erase(lightname);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::remove_all
//       Access: Published
//  Description: Remove all the lights from this effect. Returns 
//               true on success.
////////////////////////////////////////////////////////////////////
INLINE bool PolylightEffect::
remove_all() {
  LIGHTGROUP::const_iterator light_iter;
  for (light_iter = _lightgroup.begin(); light_iter != _lightgroup.end(); light_iter++){
    string lightname = light_iter->first;
    _lightgroup.erase(lightname);
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::set_weight
//       Access: Published
//  Description: weight is a constant you add (generally 1 and above)
//               to make the colorscale brighten the existing color
////////////////////////////////////////////////////////////////////
INLINE bool PolylightEffect::
set_weight(float w) {
  // nassertr(w >= 0.0 && w <= 1.0 ,false);
  _weight = w;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_weight
//       Access: Published
//  Description: Get the weight value
////////////////////////////////////////////////////////////////////
INLINE float PolylightEffect:: 
get_weight() const {
  return _weight;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::set_contrib
//       Access: Published
//  Description: Set the effect contribution type
//               The effects of all the light colors are finally 
//               divided by the number of lights. The variable
//               _contribution_type is a string that controls how
//               this division occurs.
//               "proximal" : A light only contributes if the node 
//				 is inside its volume
//               "all" : All lights added to the effect are used in
//               division irrespective of their light volumes
////////////////////////////////////////////////////////////////////
INLINE bool PolylightEffect::
set_contrib(PolylightEffect::Contrib_Type type) {
  nassertr(type == CALL || type == CPROXIMAL,false);
  _contribution_type = type;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_contrib
//       Access: Published
//  Description: Returns CALL or CPROXIMAL
////////////////////////////////////////////////////////////////////
INLINE PolylightEffect::Contrib_Type PolylightEffect::
get_contrib() const {
  return _contribution_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::set_effect_center
//       Access: Published
//  Description: Set the center point of the effect...generally 0,0,0
//               and hence the pivot point of the model
////////////////////////////////////////////////////////////////////
INLINE void PolylightEffect::
set_effect_center(LPoint3f effect_center) {
  _effect_center = effect_center;
}


////////////////////////////////////////////////////////////////////
//     Function: PolylightEffect::get_effect_center
//       Access: Published
//  Description: Return the value of the _effect_center
////////////////////////////////////////////////////////////////////
INLINE LPoint3f PolylightEffect::
get_effect_center() const {
  return _effect_center;
}
