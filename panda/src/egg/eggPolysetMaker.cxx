// Filename: eggPolysetMaker.cxx
// Created by:  drose (20Jun01)
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

#include "eggPolysetMaker.h"
#include "eggPolygon.h"

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

  if (pa->has_texture() != pb->has_texture()) {
    return ((int)pa->has_texture() < (int)pb->has_texture());
  }
  if (pa->has_texture()) {
    return (pa->get_texture()->sorts_less_than(*pb->get_texture(), ~EggTexture::E_tref_name));
  }
  if (pa->has_material() != pb->has_material()) {
    return ((int)pa->has_material() < (int)pb->has_material());
  }
  if (pa->has_material()) {
    return (pa->get_material()->sorts_less_than(*pb->get_material(), ~EggMaterial::E_mref_name));
  }
  if (pa->get_bface_flag() != pb->get_bface_flag()) {
    return ((int)pa->get_bface_flag() < (int)pb->get_bface_flag());
  }

  return false;
}
