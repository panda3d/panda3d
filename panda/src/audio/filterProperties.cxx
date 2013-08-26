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
add_filter(FilterType t, PN_stdfloat a, PN_stdfloat b, PN_stdfloat c, PN_stdfloat d,
	                     PN_stdfloat e, PN_stdfloat f, PN_stdfloat g, PN_stdfloat h,
	                     PN_stdfloat i, PN_stdfloat j, PN_stdfloat k, PN_stdfloat l,
	                     PN_stdfloat m, PN_stdfloat n) {
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
  conf._i = i;
  conf._j = j;
  conf._k = k;
  conf._l = l;
  conf._m = m;
  conf._n = n;
  _config.push_back(conf);
}

