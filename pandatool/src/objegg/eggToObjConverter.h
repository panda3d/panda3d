/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggToObjConverter.h
 * @author drose
 * @date 2012-12-19
 */

#ifndef EGGTOOBJCONVERTER_H
#define EGGTOOBJCONVERTER_H

#include "pandatoolbase.h"

#include "eggToSomethingConverter.h"
#include "eggVertexPool.h"
#include "eggGroup.h"

/**
 * Convert an obj file to egg data.
 */
class EggToObjConverter : public EggToSomethingConverter {
public:
  EggToObjConverter();
  EggToObjConverter(const EggToObjConverter &copy);
  ~EggToObjConverter();

  virtual EggToSomethingConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;

  virtual bool write_file(const Filename &filename);

private:
  typedef pmap<LVecBase4d, int> UniqueVertices;
  class VertexDef {
  public:
    VertexDef();
    int _vert3_index;
    int _vert4_index;
    int _uv2_index;
    int _uv3_index;
    int _norm_index;
  };
  typedef pmap<EggVertex *, VertexDef> VertexMap;

  bool process(const Filename &filename);

  void collect_vertices(EggNode *egg_node);
  void write_faces(std::ostream &out, EggNode *egg_node);
  void write_group_reference(std::ostream &out, EggNode *egg_node);
  void get_group_name(std::string &group_name, EggGroupNode *egg_group);

  void record_vertex(EggVertex *vertex);
  int record_unique(UniqueVertices &unique, const LVecBase4d &vec);
  int record_unique(UniqueVertices &unique, const LVecBase3d &vec);
  int record_unique(UniqueVertices &unique, const LVecBase2d &vec);
  int record_unique(UniqueVertices &unique, double pos);

  void write_vertices(std::ostream &out, const std::string &prefix, int num_components,
                      const UniqueVertices &unique);

private:
  UniqueVertices _unique_vert3, _unique_vert4, _unique_uv2, _unique_uv3, _unique_norm;
  VertexMap _vmap;
  EggGroupNode *_current_group;
};

#endif
