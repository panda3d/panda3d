// Filename: filterProperties.cxx
// Created by: jyelon (01Aug2007)
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

#include "filterProperties.h"

TypeHandle FilterProperties::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: FilterProperties::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
FilterProperties::
FilterProperties()
{
}

////////////////////////////////////////////////////////////////////
//     Function: FilterProperties::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
FilterProperties::
~FilterProperties() {
}

////////////////////////////////////////////////////////////////////
//     Function: FilterProperties::add_filter
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void FilterProperties::
add_filter(FilterType t, float a, float b, float c, float d, float e, float f, float g, float h) {
  FilterConfig conf;
  conf._type = t;
  conf._a = a;
  conf._b = b;
  conf._c = c;
  conf._d = d;
  conf._e = e;
  conf._f = f;
  conf._g = g;
  conf._h = h;
  _config.push_back(conf);
}

