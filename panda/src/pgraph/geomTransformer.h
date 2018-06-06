/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomTransformer.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef GEOMTRANSFORMER_H
#define GEOMTRANSFORMER_H

#include "pandabase.h"

#include "luse.h"
#include "geom.h"
#include "geomVertexData.h"
#include "texMatrixAttrib.h"

class GeomNode;
class RenderState;
class InternalName;
class GeomMunger;
class Texture;

/**
 * An object specifically designed to transform the vertices of a Geom without
 * disturbing indexing or affecting any other Geoms that may share the same
 * vertex arrays, and without needlessly wasting memory when different Geoms
 * sharing the same vertex arrays are transformed by the same amount.
 *
 * If you create a single GeomTransformer and use it to transform a number of
 * different Geoms by various transformations, then those Geoms which happen
 * to share the same arrays and are transformed by the same amounts will still
 * share the same arrays as each other (but different from the original
 * arrays).
 */
class EXPCL_PANDA_PGRAPH GeomTransformer {
public:
  GeomTransformer();
  GeomTransformer(const GeomTransformer &copy);
  ~GeomTransformer();

  INLINE int get_max_collect_vertices() const;
  INLINE void set_max_collect_vertices(int max_collect_vertices);

  void register_vertices(Geom *geom, bool might_have_unused);
  void register_vertices(GeomNode *node, bool might_have_unused);

  bool transform_vertices(Geom *geom, const LMatrix4 &mat);
  bool transform_vertices(GeomNode *node, const LMatrix4 &mat);

  bool transform_texcoords(Geom *geom, const InternalName *from_name,
                           InternalName *to_name, const LMatrix4 &mat);
  bool transform_texcoords(GeomNode *node, const InternalName *from_name,
                           InternalName *to_name, const LMatrix4 &mat);

  bool set_color(Geom *geom, const LColor &color);
  bool set_color(GeomNode *node, const LColor &color);

  bool transform_colors(Geom *geom, const LVecBase4 &scale);
  bool transform_colors(GeomNode *node, const LVecBase4 &scale);

  bool apply_texture_colors(Geom *geom, TextureStage *ts, Texture *tex,
                            const TexMatrixAttrib *tma,
                            const LColor &base_color, bool keep_vertex_color);
  bool apply_texture_colors(GeomNode *node, const RenderState *state);

  bool apply_state(GeomNode *node, const RenderState *state);

  bool set_format(Geom *geom, const GeomVertexFormat *new_format);
  bool remove_column(Geom *geom, const InternalName *column);
  bool remove_column(GeomNode *node, const InternalName *column);

  bool make_compatible_state(GeomNode *node);

  bool reverse_normals(Geom *geom);
  bool doubleside(GeomNode *node);
  bool reverse(GeomNode *node);

  void finish_apply();

  int collect_vertex_data(Geom *geom, int collect_bits, bool format_only);
  int collect_vertex_data(GeomNode *node, int collect_bits, bool format_only);
  int finish_collect(bool format_only);

  PT(Geom) premunge_geom(const Geom *geom, GeomMunger *munger);

private:
  int _max_collect_vertices;

  typedef pvector<PT(Geom) > GeomList;

  // Keeps track of the Geoms that are associated with a particular
  // GeomVertexData.  Also tracks whether the vertex data might have unused
  // vertices because of our actions.
  class VertexDataAssoc {
  public:
    INLINE VertexDataAssoc();
    bool _might_have_unused;
    GeomList _geoms;
    void remove_unused_vertices(const GeomVertexData *vdata);
  };
  typedef pmap<CPT(GeomVertexData), VertexDataAssoc> VertexDataAssocMap;
  VertexDataAssocMap _vdata_assoc;

  // Corresponds to a new GeomVertexData created as needed during an apply
  // operation.
  class NewVertexData {
  public:
    CPT(GeomVertexData) _vdata;
  };

  // The table of GeomVertexData objects that have been transformed by a
  // particular matrix.
  class SourceVertices {
  public:
    INLINE bool operator < (const SourceVertices &other) const;

    LMatrix4 _mat;
    CPT(GeomVertexData) _vertex_data;
  };
  typedef pmap<SourceVertices, NewVertexData> NewVertices;
  NewVertices _vertices;

  // The table of GeomVertexData objects whose texture coordinates have been
  // transformed by a particular matrix.
  class SourceTexCoords {
  public:
    INLINE bool operator < (const SourceTexCoords &other) const;

    LMatrix4 _mat;
    CPT(InternalName) _from;
    CPT(InternalName) _to;
    CPT(GeomVertexData) _vertex_data;
  };
  typedef pmap<SourceTexCoords, NewVertexData> NewTexCoords;
  NewTexCoords _texcoords;

  // The table of GeomVertexData objects whose colors have been modified.
  class SourceColors {
  public:
    INLINE bool operator < (const SourceColors &other) const;

    LVecBase4 _color;
    CPT(GeomVertexData) _vertex_data;
  };
  typedef pmap<SourceColors, NewVertexData> NewColors;

  // We have two concepts of colors: the "fixed" colors, which are slapped in
  // as a complete replacement of the original colors (e.g.  via a
  // ColorAttrib), and the "transformed" colors, which are modified from the
  // original colors (e.g.  via a ColorScaleAttrib).
  NewColors _fcolors, _tcolors;

  // The table of GeomVertexData objects whose texture colors have been
  // applied.
  class SourceTextureColors {
  public:
    INLINE bool operator < (const SourceTextureColors &other) const;

    TextureStage *_ts;
    Texture *_tex;
    const TexMatrixAttrib *_tma;
    LColor _base_color;
    bool _keep_vertex_color;
    CPT(GeomVertexData) _vertex_data;
  };
  typedef pmap<SourceTextureColors, NewVertexData> NewTextureColors;
  NewTextureColors _tex_colors;

  // The table of GeomVertexData objects whose vertex formats have been
  // modified.  For set_format(): record (format + vertex_data) ->
  // vertex_data.
  class SourceFormat {
  public:
    INLINE bool operator < (const SourceFormat &other) const;

    CPT(GeomVertexFormat) _format;
    CPT(GeomVertexData) _vertex_data;
  };
  typedef pmap<SourceFormat, NewVertexData> NewFormat;
  NewFormat _format;

  // The table of GeomVertexData objects whose normals have been reversed.
  typedef pmap<CPT(GeomVertexData), NewVertexData> ReversedNormals;
  ReversedNormals _reversed_normals;

  class NewCollectedKey {
  public:
    INLINE bool operator < (const NewCollectedKey &other) const;

    std::string _name;
    CPT(GeomVertexFormat) _format;
    Geom::UsageHint _usage_hint;
    Geom::AnimationType _animation_type;
  };

  class SourceData {
  public:
    const GeomVertexData *_vdata;
    int _num_vertices;
  };
  typedef pvector<SourceData> SourceDatas;
  class SourceGeom {
  public:
    Geom *_geom;
    int _vertex_offset;
  };
  typedef pvector<SourceGeom> SourceGeoms;
  class NewCollectedData {
  public:
    ALLOC_DELETED_CHAIN(NewCollectedData);

    NewCollectedData(const GeomVertexData *source_data);
    void add_source_data(const GeomVertexData *source_data);
    int apply_format_only_changes();
    int apply_collect_changes();

    CPT(GeomVertexFormat) _new_format;
    std::string _vdata_name;
    GeomEnums::UsageHint _usage_hint;
    SourceDatas _source_datas;
    SourceGeoms _source_geoms;
    int _num_vertices;

  private:
    // These are used just during apply_changes().
    void append_vdata(const GeomVertexData *vdata, int vertex_offset);
    void update_geoms();

    typedef vector_int IndexMap;

    PT(GeomVertexData) _new_data;
    PT(TransformBlendTable) _new_btable;
    SparseArray _new_btable_rows;

    // We need a TypeHandle just for ALLOC_DELETED_CHAIN.
  public:
    static TypeHandle get_class_type() {
      return _type_handle;
    }
    static void init_type() {
      register_type(_type_handle, "GeomTransformer::NewCollectedData");
    }

  private:
    static TypeHandle _type_handle;
  };
  typedef pvector<NewCollectedData *> NewCollectedList;
  typedef pmap<NewCollectedKey, NewCollectedData *> NewCollectedMap;
  NewCollectedList _new_collected_list;
  NewCollectedMap _new_collected_map;

  class AlreadyCollectedData {
  public:
    NewCollectedData *_ncd;
    int _vertex_offset;
  };
  typedef pmap<CPT(GeomVertexData), AlreadyCollectedData> AlreadyCollectedMap;
  AlreadyCollectedMap _already_collected_map;

  static PStatCollector _apply_vertex_collector;
  static PStatCollector _apply_texcoord_collector;
  static PStatCollector _apply_set_color_collector;
  static PStatCollector _apply_scale_color_collector;
  static PStatCollector _apply_texture_color_collector;
  static PStatCollector _apply_set_format_collector;

public:
  static void init_type() {
    NewCollectedData::init_type();
  }
};

#include "geomTransformer.I"

#endif
