// Filename: geomBinNormal.cxx
// Created by:  drose (13Apr00)
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

#include "geomBinNormal.h"
#include "geomBinUnsorted.h"
#include "geomBinBackToFront.h"

#include <transparencyTransition.h>
#include <transparencyAttribute.h>
#include <nodeAttributes.h>

TypeHandle GeomBinNormal::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: GeomBinNormal::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomBinNormal::
GeomBinNormal(const string &name) :
  GeomBinGroup(name)
{
  GeomBinUnsorted *opaque = new GeomBinUnsorted("opaque");
  GeomBinBackToFront *transparent = new GeomBinBackToFront("transparent");
  opaque->set_sort(20);
  transparent->set_sort(30);

  add_sub_bin(opaque);
  add_sub_bin(transparent);
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinNormal::Constructor
//       Access: Public, Virtual
//  Description: Identifies the particular sub-bin the indicated
//               CullState should be assigned to.
////////////////////////////////////////////////////////////////////
int GeomBinNormal::
choose_bin(CullState *cs) const {
  bool is_transparent = false;

  TransparencyAttribute *trans_attrib;
  if (get_attribute_into(trans_attrib, cs->get_attributes(),
                         TransparencyTransition::get_class_type())) {
    is_transparent =
      (trans_attrib->get_mode() != TransparencyProperty::M_none);
  }

  return is_transparent ? 1 : 0;
}
