// Filename: baseParticleEmitter.I
// Created by:  charles (26Jun00)
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
//    Function : set_emission_type
//      Access : Public
// Description : emission type assignment
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleEmitter::
set_emission_type(emissionType et) {
  _emission_type = et;
}

////////////////////////////////////////////////////////////////////
//    Function : get_emission_type
//      Access : Public
// Description : emission type query
////////////////////////////////////////////////////////////////////
INLINE BaseParticleEmitter::emissionType BaseParticleEmitter::
get_emission_type() const {
  return _emission_type;
}

////////////////////////////////////////////////////////////////////
//    Function : set_explicit_launch_vector
//      Access : Public
// Description : assignment of explicit emission launch vector
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleEmitter::
set_explicit_launch_vector(const LVector3f& elv) {
  _explicit_launch_vector = elv;
}

////////////////////////////////////////////////////////////////////
//    Function : get_explicit_launch_vector
//      Access : Public
// Description : query for explicit emission launch vector
////////////////////////////////////////////////////////////////////
INLINE LVector3f BaseParticleEmitter::
get_explicit_launch_vector() const {
  return _explicit_launch_vector;
}

////////////////////////////////////////////////////////////////////
//    Function : set_radiate_origin
//      Access : Public
// Description : assignment of radiate emission origin point
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleEmitter::
set_radiate_origin(const LPoint3f& ro) {
  _radiate_origin = ro;
}

////////////////////////////////////////////////////////////////////
//    Function : get_radiate_origin
//      Access : Public
// Description : query for explicit emission launch vector
////////////////////////////////////////////////////////////////////
INLINE LPoint3f BaseParticleEmitter::
get_radiate_origin() const {
  return _radiate_origin;
}

////////////////////////////////////////////////////////////////////
//    Function : set_amplitude
//      Access : Public
// Description : amplitude assignment
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleEmitter::
set_amplitude(float a) {
  _amplitude = a;
}

////////////////////////////////////////////////////////////////////
//    Function : get_amplitude
//      Access : Public
// Description : amplitude query
////////////////////////////////////////////////////////////////////
INLINE float BaseParticleEmitter::
get_amplitude() const {
  return _amplitude;
}

////////////////////////////////////////////////////////////////////
//    Function : set_amplitude_spread
//      Access : Public
// Description : amplitude spread assignment
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleEmitter::
set_amplitude_spread(float as) {
  _amplitude_spread = as;
}

////////////////////////////////////////////////////////////////////
//    Function : get_amplitude_spread
//      Access : Public
// Description : amplitude spread query
////////////////////////////////////////////////////////////////////
INLINE float BaseParticleEmitter::
get_amplitude_spread() const {
  return _amplitude_spread;
}

////////////////////////////////////////////////////////////////////
//    Function : set_offset_force
//      Access : Public
// Description : user-defined force
////////////////////////////////////////////////////////////////////
INLINE void BaseParticleEmitter::
set_offset_force(const LVector3f& of) {
  _offset_force = of;
}

////////////////////////////////////////////////////////////////////
//    Function : get_offset_force
//      Access : Public
// Description : user-defined force
////////////////////////////////////////////////////////////////////
INLINE LVector3f BaseParticleEmitter::
get_offset_force() const {
  return _offset_force;
}
