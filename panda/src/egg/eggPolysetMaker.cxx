// Filename: eggPolysetMaker.cxx
// Created by:  drose (20Jun01)
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

#include "eggPolysetMaker.h"
#include "eggPolygon.h"

////////////////////////////////////////////////////////////////////
//     Function: EggPolysetMaker::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggPolysetMaker::
EggPolysetMaker() {
  _properties = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolysetMaker::set_properties
//       Access: Public
//  Description: Sets the set of properties that determines which
//               polygons are allowed to be grouped together into a
//               single polyset.  This is the bitwise 'or' of all the
//               properties that matter.  If this is 0, all polygons
//               (within a given group) will be lumped into a common
//               polyset regardless of their properties.
////////////////////////////////////////////////////////////////////
void EggPolysetMaker::
set_properties(int properties) {
  _properties = properties;
}

////////////////////////////////////////////////////////////////////
//     Function: EggPolysetMaker::get_bin_number
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
int EggPolysetMaker::
get_bin_number(const EggNode *node) {
  if (node->is_of_type(EggPolygon::get_class_type())) {
    return (int)BN_polyset;
  }

  return (int)BN_none;
}


////////////////////////////////////////////////////////////////////
//     Function: EggPolysetMaker::sorts_less
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
bool EggPolysetMaker::
sorts_less(int bin_number, const EggNode *a, const EggNode *b) {
  nassertr((BinNumber)bin_number == BN_polyset, false);

  const EggPolygon *pa = DCAST(EggPolygon, a);
  const EggPolygon *pb = DCAST(EggPolygon, b);

  if ((_properties & (P_has_texture | P_texture)) != 0) {
    if (pa->has_texture() != pb->has_texture()) {
      return ((int)pa->has_texture() < (int)pb->has_texture());
    }
  }
  if ((_properties & (P_texture)) != 0) {
    if (pa->has_texture()) {
      return (pa->get_texture()->sorts_less_than(*pb->get_texture(), ~EggTexture::E_tref_name));
    }
  }
  if ((_properties & (P_has_material | P_material)) != 0) {
    if (pa->has_material() != pb->has_material()) {
      return ((int)pa->has_material() < (int)pb->has_material());
    }
  }
  if ((_properties & (P_material)) != 0) {
    if (pa->has_material()) {
      return (pa->get_material()->sorts_less_than(*pb->get_material(), ~EggMaterial::E_mref_name));
    }
  }
  if ((_properties & (P_has_poly_color)) != 0) {
    if (pa->has_color() != pb->has_color()) {
      return ((int)pa->has_color() < (int)pb->has_color());
    }
  }
  if ((_properties & (P_poly_color)) != 0) {
    if (pa->get_color() != pb->get_color()) {
      return (pa->get_color() < pb->get_color());
    }
  }
  if ((_properties & (P_has_poly_normal)) != 0) {
    if (pa->has_normal() != pb->has_normal()) {
      return ((int)pa->has_normal() < (int)pb->has_normal());
    }
  }
  if ((_properties & (P_has_vertex_normal)) != 0) {
    bool pa_has_normal = pa->has_vertex_normal();
    bool pb_has_normal = pb->has_vertex_normal();
    if (pa_has_normal != pb_has_normal) {
      return ((int)pa_has_normal < (int)pb_has_normal);
    }
  }
  if ((_properties & (P_has_vertex_color)) != 0) {
    bool pa_has_color = pa->has_vertex_color();
    bool pb_has_color = pb->has_vertex_color();
    if (pa_has_color != pb_has_color) {
      return ((int)pa_has_color < (int)pb_has_color);
    }
  }
  if ((_properties & (P_bface)) != 0) {
    if (pa->get_bface_flag() != pb->get_bface_flag()) {
      return ((int)pa->get_bface_flag() < (int)pb->get_bface_flag());
    }
  }

  return false;
}
