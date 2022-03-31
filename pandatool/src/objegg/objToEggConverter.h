/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file objToEggConverter.h
 * @author drose
 * @date 2010-12-07
 */

#ifndef OBJTOEGGCONVERTER_H
#define OBJTOEGGCONVERTER_H

#include "pandatoolbase.h"

#include "somethingToEggConverter.h"
#include "eggVertexPool.h"
#include "eggGroup.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomPrimitive.h"
#include "geomNode.h"
#include "pandaNode.h"
#include "pvector.h"
#include "epvector.h"

/**
 * Convert an Obj file to egg data.
 */
class ObjToEggConverter : public SomethingToEggConverter {
public:
  ObjToEggConverter();
  ObjToEggConverter(const ObjToEggConverter &copy);
  ~ObjToEggConverter();

  virtual SomethingToEggConverter *make_copy();

  virtual std::string get_name() const;
  virtual std::string get_extension() const;
  virtual bool supports_compressed() const;
  virtual bool supports_convert_to_node(const LoaderOptions &options) const;

  virtual bool convert_file(const Filename &filename);
  virtual PT(PandaNode) convert_to_node(const LoaderOptions &options, const Filename &filename);

protected:
  bool process(const Filename &filename);
  bool process_line(const std::string &line);
  bool process_ref_plane_res(const std::string &line);

  bool process_v(vector_string &words);
  bool process_vt(vector_string &words);
  bool process_xvt(vector_string &words);
  bool process_xvc(vector_string &words);
  bool process_vn(vector_string &words);
  bool process_f(vector_string &words);
  bool process_g(vector_string &words);

  EggVertex *get_face_vertex(const std::string &face_reference);
  void generate_egg_points();

  bool process_node(const Filename &filename);
  bool process_line_node(const std::string &line);

  bool process_f_node(vector_string &words);
  bool process_g_node(vector_string &words);

  void generate_points();
  int add_synth_normal(const LVecBase3d &normal);

  // Read from the obj file.
  int _line_number;
  typedef epvector<LVecBase4d> Vec4Table;
  typedef epvector<LVecBase3d> Vec3Table;
  typedef epvector<LVecBase2d> Vec2Table;
  typedef pmap<LVecBase3d, int> UniqueVec3Table;

  Vec4Table _v_table;
  Vec3Table _vn_table, _rgb_table;
  Vec3Table _vt_table;
  Vec2Table _xvt_table;
  Vec3Table _synth_vn_table;
  UniqueVec3Table _unique_synth_vn_table;
  LVecBase2d _ref_plane_res;
  bool _v4_given, _vt3_given;
  bool _f_given;

  pset<std::string> _ignored_tags;

  // Structures filled when creating an egg file.
  PT(EggVertexPool) _vpool;
  PT(EggGroup) _root_group;
  EggGroup *_current_group;

  // Structures filled when creating a PandaNode directly.
  PT(PandaNode) _root_node;

  class VertexEntry {
  public:
    VertexEntry();
    VertexEntry(const ObjToEggConverter *converter, const std::string &obj_vertex);

    INLINE bool operator < (const VertexEntry &other) const;
    INLINE bool operator == (const VertexEntry &other) const;
    INLINE bool matches_except_normal(const VertexEntry &other) const;

    // The 1-based vertex, texcoord, and normal index numbers appearing in the
    // obj file for this vertex.  0 if the index number is not given.
    int _vi, _vti, _vni;

    // The 1-based index number to the synthesized normal, if needed.
    int _synth_vni;
  };
  typedef pmap<VertexEntry, int> UniqueVertexEntries;
  typedef pvector<VertexEntry> VertexEntries;

  class VertexData {
  public:
    VertexData(PandaNode *parent, const std::string &name);

    int add_vertex(const ObjToEggConverter *converter, const VertexEntry &entry);
    void add_triangle(const ObjToEggConverter *converter, const VertexEntry &v0,
                      const VertexEntry &v1, const VertexEntry &v2,
                      int synth_vni);
    void close_geom(const ObjToEggConverter *converter);

    PT(PandaNode) _parent;
    std::string _name;
    PT(GeomNode) _geom_node;

    PT(GeomPrimitive) _prim;
    VertexEntries _entries;
    UniqueVertexEntries _unique_entries;

    bool _v4_given, _vt3_given;
    bool _vt_given, _rgb_given, _vn_given;
  };

  VertexData *_current_vertex_data;

  friend class VertexData;
};

#include "objToEggConverter.I"

#endif
