// Filename: cLwoPoints.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoPoints.h"
#include "lwoToEggConverter.h"
#include "cLwoLayer.h"

#include <string_utils.h>

////////////////////////////////////////////////////////////////////
//     Function: CLwoPoints::make_egg
//       Access: Public
//  Description: Creates the egg structures associated with this
//               Lightwave object.
////////////////////////////////////////////////////////////////////
void CLwoPoints::
make_egg() {
  // Generate a vpool name based on the layer index, for lack of
  // anything better.
  string vpool_name = "layer" + format_string(_layer->get_number());
  _egg_vpool = new EggVertexPool(vpool_name);
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoPoints::connect_egg
//       Access: Public
//  Description: Connects all the egg structures together.
////////////////////////////////////////////////////////////////////
void CLwoPoints::
connect_egg() {
  if (!_egg_vpool->empty()) {
    _layer->_egg_group->add_child(_egg_vpool.p());
  }
}

