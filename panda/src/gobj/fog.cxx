// Filename: fog.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include <pandabase.h>

#include "fog.h"

#include <mathNumbers.h>

#include <stddef.h>

////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////
TypeHandle Fog::_type_handle;

ostream &
operator << (ostream &out, Fog::Mode mode) {
  switch (mode) {
  case Fog::M_linear:
    return out << "linear";

  case Fog::M_exponential:
    return out << "exponential";

  case Fog::M_super_exponential:
    return out << "super_exponential";

  case Fog::M_spline:
    return out << "spline";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Fog::Fog(Mode mode, int hardware_bits) {
  _hardware_bits = hardware_bits;
  set_mode(mode);
  set_color(Colorf(0.0, 0.0, 0.0, 1.0));
  set_range(0.0f, 100.0f);
  set_offsets(0.0f, 0.0f);
  compute_density();
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Destructor 
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Fog::~Fog(void) {
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::set_mode
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
void Fog::compute_density(void) {
  _density = 1.0f;
  float opaque_multiplier;
  switch (_mode) {
  case M_linear: 
    break;
  case M_exponential:
    // Multiplier = ln(2^bits)
    opaque_multiplier = MathNumbers::ln2 * _hardware_bits;
    _density = opaque_multiplier / (_opaque + _opaque_offset);
    break;
  case M_super_exponential:
    // Multiplier = ln(squrt(2^bits))
    opaque_multiplier = 0.5f * MathNumbers::ln2 * _hardware_bits;
    opaque_multiplier *= opaque_multiplier;
    _density = opaque_multiplier / (_opaque + _opaque_offset);
    break;
  case M_spline:
    // *** What's this?
    break;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: Fog::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Fog::
output(ostream &out) const {
  out << "fog:" << _mode;
  switch (_mode) {
  case M_linear: 
    break;
  case M_exponential:
  case M_super_exponential:
    out << "(" << _hardware_bits << "," << _density
	<< "," << _opaque << "," << _opaque_offset << ")";
    break;
  case M_spline:
    break;
  };
}
