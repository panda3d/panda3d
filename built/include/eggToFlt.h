/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToFlt.h
 * @author drose
 * @date 2003-10-01
 */

#ifndef EGGTOFLT_H
#define EGGTOFLT_H

#include "pandatoolbase.h"

#include "eggToSomething.h"
#include "fltHeader.h"
#include "fltGeometry.h"
#include "pointerTo.h"
#include "pmap.h"
#include "vector_string.h"

class EggGroup;
class EggVertex;
class EggPrimitive;
class EggTexture;
class EggTransform;
class FltVertex;
class FltBead;
class FltTexture;

/**
 * A program to read an egg file and write a flt file.
 */
class EggToFlt : public EggToSomething {
public:
  EggToFlt();

  void run();

private:
  static bool dispatch_attr(const std::string &opt, const std::string &arg, void *var);

  void traverse(EggNode *egg_node, FltBead *flt_node,
                FltGeometry::BillboardType billboard);
  void convert_primitive(EggPrimitive *egg_primitive, FltBead *flt_node,
                         FltGeometry::BillboardType billboard);
  void convert_group(EggGroup *egg_group, FltBead *flt_node,
                     FltGeometry::BillboardType billboard);
  void apply_transform(EggTransform *egg_transform, FltBead *flt_node);
  void apply_egg_syntax(const std::string &egg_syntax, FltRecord *flt_record);
  FltVertex *get_flt_vertex(EggVertex *egg_vertex, EggNode *context);
  FltTexture *get_flt_texture(EggTexture *egg_texture);

  FltHeader::AttrUpdate _auto_attr_update;

  PT(FltHeader) _flt_header;

  typedef pmap<EggVertex *, FltVertex *> VertexMap;
  typedef pmap<const LMatrix4d *, VertexMap> VertexMapPerFrame;
  VertexMapPerFrame _vertex_map_per_frame;

  typedef pmap<Filename, FltTexture *> TextureMap;
  TextureMap _texture_map;
};

#endif
