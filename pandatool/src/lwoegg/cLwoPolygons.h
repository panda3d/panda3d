/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cLwoPolygons.h
 * @author drose
 * @date 2001-04-25
 */

#ifndef CLWOPOLYGONS_H
#define CLWOPOLYGONS_H

#include "pandatoolbase.h"

#include "lwoPolygons.h"
#include "eggGroup.h"
#include "pointerTo.h"

#include "pmap.h"

class LwoToEggConverter;
class CLwoPoints;
class CLwoSurface;
class LwoTags;
class LwoPolygonTags;
class LwoDiscontinuousVertexMap;

/**
 * This class is a wrapper around LwoPolygons and stores additional
 * information useful during the conversion-to-egg process.
 */
class CLwoPolygons {
public:
  INLINE CLwoPolygons(LwoToEggConverter *converter,
                      const LwoPolygons *polygons,
                      CLwoPoints *points);

  void add_ptags(const LwoPolygonTags *lwo_ptags, const LwoTags *tags);
  void add_vmad(const LwoDiscontinuousVertexMap *lwo_vmad);

  CLwoSurface *get_surface(int polygon_index) const;
  bool get_uv(const std::string &uv_name, int pi, int vi, LPoint2 &uv) const;

  void make_egg();
  void connect_egg();

  LwoToEggConverter *_converter;
  CPT(LwoPolygons) _polygons;
  CLwoPoints *_points;
  PT(EggGroup) _egg_group;

  const LwoTags *_tags;
  typedef pmap<IffId, const LwoPolygonTags *> PTags;
  PTags _ptags;

  const LwoPolygonTags *_surf_ptags;

  // There might be named maps associated with the polygons to bring a per-
  // polygon mapping to the UV's.
  typedef pmap<std::string, const LwoDiscontinuousVertexMap *> VMad;
  VMad _txuv;

private:
  void make_faces();
};

#include "cLwoPolygons.I"

#endif
