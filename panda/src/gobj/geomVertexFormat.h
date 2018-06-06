/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomVertexFormat.h
 * @author drose
 * @date 2005-03-07
 */

#ifndef GEOMVERTEXFORMAT_H
#define GEOMVERTEXFORMAT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "geomVertexAnimationSpec.h"
#include "geomVertexArrayFormat.h"
#include "geomEnums.h"
#include "internalName.h"
#include "luse.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pset.h"
#include "pvector.h"
#include "indirectCompareTo.h"
#include "lightReMutex.h"

class FactoryParams;
class GeomVertexData;
class GeomMunger;

/**
 * This class defines the physical layout of the vertex data stored within a
 * Geom.  The layout consists of a list of named columns, each of which has a
 * numeric type and a size.
 *
 * The columns are typically interleaved within a single array, but they may
 * also be distributed among multiple different arrays; at the extreme, each
 * column may be alone within its own array (which amounts to a parallel-array
 * definition).
 *
 * Thus, a GeomVertexFormat is really a list of GeomVertexArrayFormats, each
 * of which contains a list of columns.  However, a particular column name
 * should not appear more than once in the format, even between different
 * arrays.
 *
 * There are a handful of standard pre-defined GeomVertexFormat objects, or
 * you may define your own as needed.  You may record any combination of
 * standard and/or user-defined columns in your custom GeomVertexFormat
 * constructions.
 */
class EXPCL_PANDA_GOBJ GeomVertexFormat final : public TypedWritableReferenceCount, public GeomEnums {
PUBLISHED:
  GeomVertexFormat();
  GeomVertexFormat(const GeomVertexArrayFormat *array_format);
  GeomVertexFormat(const GeomVertexFormat &copy);
  void operator = (const GeomVertexFormat &copy);
  virtual ~GeomVertexFormat();

  virtual bool unref() const;

  INLINE bool is_registered() const;
  INLINE static CPT(GeomVertexFormat) register_format(const GeomVertexFormat *format);
  INLINE static CPT(GeomVertexFormat) register_format(const GeomVertexArrayFormat *format);
  MAKE_PROPERTY(registered, is_registered);

  INLINE const GeomVertexAnimationSpec &get_animation() const;
  INLINE void set_animation(const GeomVertexAnimationSpec &animation);
  MAKE_PROPERTY(animation, get_animation, set_animation);

  CPT(GeomVertexFormat) get_post_animated_format() const;
  CPT(GeomVertexFormat) get_union_format(const GeomVertexFormat *other) const;

  INLINE size_t get_num_arrays() const;
  INLINE const GeomVertexArrayFormat *get_array(size_t array) const;
  MAKE_SEQ(get_arrays, get_num_arrays, get_array);
  GeomVertexArrayFormat *modify_array(size_t array);
  void set_array(size_t array, const GeomVertexArrayFormat *format);
  void remove_array(size_t array);
  size_t add_array(const GeomVertexArrayFormat *array_format);
  void insert_array(size_t array, const GeomVertexArrayFormat *array_format);
  void clear_arrays();
  void remove_empty_arrays();

  size_t get_num_columns() const;
  int get_array_with(size_t i) const;
  const GeomVertexColumn *get_column(size_t i) const;

  int get_array_with(const InternalName *name) const;
  const GeomVertexColumn *get_column(const InternalName *name) const;
  INLINE bool has_column(const InternalName *name) const;
  const InternalName *get_column_name(size_t i) const;

  MAKE_SEQ(get_columns, get_num_columns, get_column);

  void remove_column(const InternalName *name, bool keep_empty_array = false);
  void pack_columns();
  void align_columns_for_animation();
  void maybe_align_columns_for_animation();

  INLINE size_t get_num_points() const;
  INLINE const InternalName *get_point(size_t n) const;
  MAKE_SEQ(get_points, get_num_points, get_point);

  INLINE size_t get_num_vectors() const;
  INLINE const InternalName *get_vector(size_t n) const;
  MAKE_SEQ(get_vectors, get_num_vectors, get_vector);

  INLINE size_t get_num_texcoords() const;
  INLINE const InternalName *get_texcoord(size_t n) const;
  MAKE_SEQ(get_texcoords, get_num_texcoords, get_texcoord);

  INLINE size_t get_num_morphs() const;
  INLINE const InternalName *get_morph_slider(size_t n) const;
  INLINE const InternalName *get_morph_base(size_t n) const;
  INLINE const InternalName *get_morph_delta(size_t n) const;
  MAKE_SEQ(get_morph_sliders, get_num_morphs, get_morph_slider);
  MAKE_SEQ(get_morph_bases, get_num_morphs, get_morph_base);
  MAKE_SEQ(get_morph_deltas, get_num_morphs, get_morph_delta);

  MAKE_SEQ_PROPERTY(arrays, get_num_arrays, get_array, set_array, remove_array, insert_array);
  MAKE_SEQ_PROPERTY(points, get_num_points, get_point);
  MAKE_SEQ_PROPERTY(vectors, get_num_vectors, get_vector);

  // We also define this as a mapping interface, for lookups by name.
  MAKE_MAP_PROPERTY(columns, has_column, get_column);
  MAKE_MAP_KEYS_SEQ(columns, get_num_columns, get_column_name);

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;
  void write_with_data(std::ostream &out, int indent_level,
                       const GeomVertexData *data) const;

  INLINE static const GeomVertexFormat *get_empty();

  // Some standard vertex formats.  No particular requirement to use one of
  // these, but the DirectX renderers can use these formats directly, whereas
  // any other format will have to be converted first.
  INLINE static const GeomVertexFormat *get_v3();
  INLINE static const GeomVertexFormat *get_v3n3();
  INLINE static const GeomVertexFormat *get_v3t2();
  INLINE static const GeomVertexFormat *get_v3n3t2();

  // These formats, with the DirectX-style packed color, may not be supported
  // directly by OpenGL.  If you use them and the driver does not support
  // them, the GLGraphicsStateGuardian will automatically convert to native
  // OpenGL form (with a small runtime overhead).
  INLINE static const GeomVertexFormat *get_v3cp();
  INLINE static const GeomVertexFormat *get_v3cpt2();
  INLINE static const GeomVertexFormat *get_v3n3cp();
  INLINE static const GeomVertexFormat *get_v3n3cpt2();

  // These formats, with an OpenGL-style four-byte color, are not supported
  // directly by DirectX.  If you use them, the DXGraphicsStateGuardian will
  // automatically convert to DirectX form (with a larger runtime overhead,
  // since DirectX8, and old DirectX9 drivers, require everything to be
  // interleaved together).
  INLINE static const GeomVertexFormat *get_v3c4();
  INLINE static const GeomVertexFormat *get_v3c4t2();
  INLINE static const GeomVertexFormat *get_v3n3c4();
  INLINE static const GeomVertexFormat *get_v3n3c4t2();

public:
  bool get_array_info(const InternalName *name,
                      int &array_index,
                      const GeomVertexColumn *&column) const;

  INLINE int get_vertex_array_index() const;
  INLINE const GeomVertexColumn *get_vertex_column() const;
  INLINE int get_normal_array_index() const;
  INLINE const GeomVertexColumn *get_normal_column() const;
  INLINE int get_color_array_index() const;
  INLINE const GeomVertexColumn *get_color_column() const;

  int compare_to(const GeomVertexFormat &other) const;

private:
  class Registry;
  INLINE static Registry *get_registry();
  static void make_registry();

  void do_register();
  void do_unregister();

  bool _is_registered;

  GeomVertexAnimationSpec _animation;

  typedef pvector< PT(GeomVertexArrayFormat) > Arrays;
  Arrays _arrays;

  class DataTypeRecord {
  public:
    int _array_index;
    int _column_index;
  };

  typedef pmap<const InternalName *, DataTypeRecord> DataTypesByName;
  DataTypesByName _columns_by_name;

  int _vertex_array_index;
  const GeomVertexColumn *_vertex_column;
  int _normal_array_index;
  const GeomVertexColumn *_normal_column;
  int _color_array_index;
  const GeomVertexColumn *_color_column;

  typedef pvector<CPT_InternalName> Columns;
  Columns _points;
  Columns _vectors;
  Columns _texcoords;

  class MorphRecord {
  public:
    CPT_InternalName _slider;
    CPT_InternalName _base;
    CPT_InternalName _delta;
  };
  typedef pvector<MorphRecord> Morphs;
  Morphs _morphs;

  const GeomVertexFormat *_post_animated_format;

  // This is the global registry of all currently-in-use formats.
  typedef pset<GeomVertexFormat *, IndirectCompareTo<GeomVertexFormat> > Formats;
  class EXPCL_PANDA_GOBJ Registry {
  public:
    Registry();
    void make_standard_formats();

    CPT(GeomVertexFormat) register_format(GeomVertexFormat *format);
    INLINE CPT(GeomVertexFormat) register_format(GeomVertexArrayFormat *format);
    void unregister_format(GeomVertexFormat *format);

    Formats _formats;
    LightReMutex _lock;

    CPT(GeomVertexFormat) _empty;

    CPT(GeomVertexFormat) _v3;
    CPT(GeomVertexFormat) _v3n3;
    CPT(GeomVertexFormat) _v3t2;
    CPT(GeomVertexFormat) _v3n3t2;

    CPT(GeomVertexFormat) _v3cp;
    CPT(GeomVertexFormat) _v3n3cp;
    CPT(GeomVertexFormat) _v3cpt2;
    CPT(GeomVertexFormat) _v3n3cpt2;

    CPT(GeomVertexFormat) _v3c4;
    CPT(GeomVertexFormat) _v3n3c4;
    CPT(GeomVertexFormat) _v3c4t2;
    CPT(GeomVertexFormat) _v3n3c4t2;
  };

  static Registry *_registry;


public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "GeomVertexFormat",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GeomVertexFormat::Registry;
  friend class GeomMunger;
};

INLINE std::ostream &operator << (std::ostream &out, const GeomVertexFormat &obj);

#include "geomVertexFormat.I"

#endif
