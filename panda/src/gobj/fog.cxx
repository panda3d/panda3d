// Filename: fog.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
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

  case Fog::M_exponential_squared:
    return out << "exponential-squared";
  }

  return out << "**invalid**(" << (int)mode << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: Fog::Constructor
//       Access:
//  Description:
////////////////////////////////////////////////////////////////////
Fog::Fog(Mode mode, int bits_per_color_channel) {
  _bits_per_color_channel = bits_per_color_channel;
  set_mode(mode);
  set_color(Colorf(1.0f, 1.0f, 1.0f, 1.0f));
  set_range(0.0f, 100.0f);

  _density = 0.5f;
//  compute_density();
}

#if 0
// this fn tries to 'match' exponential to linear fog by computing a exponential density
// factor such that exponential fog matches linear fog at the linear fog's 'fog-end' distance.
// usefulness of this is debatable since the exponential fog that matches will be very very
// close to observer, and it's harder to guess a good end fog distance than it is to manipulate
// the [0.0,1.0] density range directly, so taking this out

////////////////////////////////////////////////////////////////////
//     Function: Fog::compute_density
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
        // attempt to compute density based on full
    opaque_multiplier = MathNumbers::ln2 * _bits_per_color_channel;
    _density = opaque_multiplier / _opaque_distance;
    break;
  case M_exponential_squared:
    // Multiplier = ln(sqrt(2^bits))
    opaque_multiplier = 0.5f * MathNumbers::ln2 * _bits_per_color_channel;
    opaque_multiplier *= opaque_multiplier;
    _density = opaque_multiplier / _opaque_distance;
    break;
  }
}

#endif

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
  case M_exponential_squared:
    out << "(" << _bits_per_color_channel << "," << _density
        << "," << _opaque_distance << ")";
    break;
  };
}
