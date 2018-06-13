/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoPoints.cxx
 * @author drose
 * @date 2001-04-25
 */

#include "cLwoPoints.h"
#include "lwoToEggConverter.h"
#include "cLwoLayer.h"

#include "pta_stdfloat.h"
#include "lwoVertexMap.h"
#include "string_utils.h"

/**
 * Associates the indicated VertexMap with the points set.  This may define
 * such niceties as UV coordinates or per-vertex color.
 */
void CLwoPoints::
add_vmap(const LwoVertexMap *lwo_vmap) {
  IffId map_type = lwo_vmap->_map_type;
  const std::string &name = lwo_vmap->_name;

  bool inserted;
  if (map_type == IffId("TXUV")) {
    inserted =
      _txuv.insert(VMap::value_type(name, lwo_vmap)).second;

  } else if (map_type == IffId("PICK")) {
    inserted =
      _pick.insert(VMap::value_type(name, lwo_vmap)).second;

  } else {
    return;
  }

  if (!inserted) {
    nout << "Multiple vertex maps on the same points of type "
         << map_type << " named " << name << "\n";
  }
}

/**
 * Returns true if there is a UV of the indicated name associated with the
 * given vertex, false otherwise.  If true, fills in uv with the value.
 */
bool CLwoPoints::
get_uv(const std::string &uv_name, int n, LPoint2 &uv) const {
  VMap::const_iterator ni = _txuv.find(uv_name);
  if (ni == _txuv.end()) {
    return false;
  }

  const LwoVertexMap *vmap = (*ni).second;
  if (vmap->_dimension != 2) {
    nout << "Unexpected dimension of " << vmap->_dimension
         << " for UV map " << uv_name << "\n";
    return false;
  }

  if (!vmap->has_value(n)) {
    return false;
  }

  PTA_stdfloat value = vmap->get_value(n);

  uv.set(value[0], value[1]);
  return true;
}

/**
 * Creates the egg structures associated with this Lightwave object.
 */
void CLwoPoints::
make_egg() {
  // Generate a vpool name based on the layer index, for lack of anything
  // better.
  std::string vpool_name = "layer" + format_string(_layer->get_number());
  _egg_vpool = new EggVertexPool(vpool_name);
}

/**
 * Connects all the egg structures together.
 */
void CLwoPoints::
connect_egg() {
  if (!_egg_vpool->empty()) {
    _layer->_egg_group->add_child(_egg_vpool.p());
  }
}
