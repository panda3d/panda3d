// Filename: geomBinNormal.cxx
// Created by:  drose (13Apr00)
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
GeomBinNormal(const string &name, CullTraverser *traverser, int sort) :
  GeomBinGroup(name, traverser, sort) 
{
  add_sub_bin(new GeomBinUnsorted("opaque"));
  add_sub_bin(new GeomBinBackToFront("transparent"));
}

////////////////////////////////////////////////////////////////////
//     Function: GeomBinNormal::Constructor
//       Access: Public
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
