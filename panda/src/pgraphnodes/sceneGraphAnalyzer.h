/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneGraphAnalyzer.h
 * @author drose
 * @date 2000-07-02
 */

#ifndef SCENEGRAPHANALYZER_H
#define SCENEGRAPHANALYZER_H

#include "pandabase.h"
#include "typedObject.h"
#include "luse.h"
#include "pmap.h"
#include "pset.h"
#include "bitArray.h"
#include "indirectCompareTo.h"

class PandaNode;
class GeomNode;
class Geom;
class GeomVertexData;
class GeomVertexFormat;
class GeomVertexArrayData;
class Texture;

/**
 * A handy class that can scrub over a scene graph and collect interesting
 * statistics on it.
 */
class EXPCL_PANDA_PGRAPHNODES SceneGraphAnalyzer {
PUBLISHED:
  SceneGraphAnalyzer();
  ~SceneGraphAnalyzer();

  enum LodMode {
    LM_lowest,
    LM_highest,
    LM_all,
    LM_none,
  };

  INLINE void set_lod_mode(LodMode lod_mode);
  INLINE LodMode get_lod_mode(LodMode lod_mode) const;

  void clear();
  void add_node(PandaNode *node);

  void write(std::ostream &out, int indent_level = 0) const;

  INLINE int get_num_nodes() const;
  INLINE int get_num_instances() const;
  INLINE int get_num_transforms() const;
  INLINE int get_num_nodes_with_attribs() const;
  INLINE int get_num_lod_nodes() const;
  INLINE int get_num_geom_nodes() const;
  INLINE int get_num_geoms() const;
  INLINE int get_num_geom_vertex_datas() const;
  INLINE int get_num_geom_vertex_formats() const;
  INLINE size_t get_vertex_data_size() const;

  INLINE int get_num_vertices() const;
  INLINE int get_num_normals() const;
  INLINE int get_num_colors() const;
  INLINE int get_num_texcoords() const;
  INLINE int get_num_tris() const;
  INLINE int get_num_lines() const;
  INLINE int get_num_points() const;
  INLINE int get_num_patches() const;

  INLINE int get_num_individual_tris() const;
  INLINE int get_num_tristrips() const;
  INLINE int get_num_triangles_in_strips() const;
  INLINE int get_num_trifans() const;
  INLINE int get_num_triangles_in_fans() const;
  INLINE int get_num_vertices_in_patches() const;

  INLINE size_t get_texture_bytes() const;

  INLINE int get_num_long_normals() const;
  INLINE int get_num_short_normals() const;
  INLINE PN_stdfloat get_total_normal_length() const;

private:
  void collect_statistics(PandaNode *node, bool under_instance);
  void collect_statistics(GeomNode *geom_node);
  void collect_statistics(const Geom *geom);
  void collect_statistics(Texture *texture);
  void collect_statistics(const GeomVertexArrayData *vadata);
  void collect_prim_statistics(const GeomVertexArrayData *vadata);

  class VDataTracker {
  public:
    BitArray _referenced_vertices;
  };

  typedef pmap<PandaNode *, int> Nodes;
  typedef pmap<CPT(GeomVertexData), VDataTracker> VDatas;
  typedef pset<CPT(GeomVertexFormat) > VFormats;
  typedef pset<CPT(GeomVertexArrayData) > VADatas;
  typedef pmap<const GeomVertexData *, int, IndirectCompareTo<GeomVertexData> > UniqueVDatas;
  typedef pmap<const GeomVertexArrayData *, int, IndirectCompareTo<GeomVertexArrayData> > UniqueVADatas;
  typedef pmap<Texture *, int> Textures;

  LodMode _lod_mode;

  Nodes _nodes;
  VDatas _vdatas;
  VFormats _vformats;
  VADatas _vadatas;
  VADatas _prim_vadatas;
  UniqueVDatas _unique_vdatas;
  UniqueVADatas _unique_vadatas;
  UniqueVADatas _unique_prim_vadatas;
  Textures _textures;

private:
  int _num_nodes;
  int _num_instances;
  int _num_transforms;
  int _num_nodes_with_attribs;
  int _num_lod_nodes;
  int _num_geom_nodes;
  int _num_geoms;
  int _num_geom_vertex_datas;
  int _num_geom_vertex_formats;
  size_t _vertex_data_size;
  size_t _prim_data_size;

  int _num_vertices;
  int _num_vertices_64;
  int _num_normals;
  int _num_colors;
  int _num_texcoords;
  int _num_tris;
  int _num_lines;
  int _num_points;
  int _num_patches;

  int _num_individual_tris;
  int _num_tristrips;
  int _num_triangles_in_strips;
  int _num_trifans;
  int _num_triangles_in_fans;
  int _num_vertices_in_patches;

  size_t _texture_bytes;

  int _num_long_normals;
  int _num_short_normals;
  PN_stdfloat _total_normal_length;
};

#include "sceneGraphAnalyzer.I"

#endif
