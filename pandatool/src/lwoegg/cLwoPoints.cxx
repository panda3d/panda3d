// Filename: cLwoPoints.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoPoints.h"
#include "lwoToEggConverter.h"
#include "cLwoLayer.h"

#include <lwoVertexMap.h>
#include <string_utils.h>

////////////////////////////////////////////////////////////////////
//     Function: CLwoPoints::add_vmap
//       Access: Public
//  Description: Associated the indicated VertexMap with the points
//               set.  This may define such niceties as UV coordinates
//               or per-vertex color.
////////////////////////////////////////////////////////////////////
void CLwoPoints::
add_vmap(const LwoVertexMap *lwo_vmap) {
  VMapNames &names = _vmap[lwo_vmap->_map_type];
  bool inserted = names.insert(VMapNames::value_type(lwo_vmap->_name, lwo_vmap)).second;
  if (!inserted) {
    nout << "Multiple vertex maps on the same points of type " 
	 << lwo_vmap->_map_type << " named " << lwo_vmap->_name << "\n";
  }
}

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

