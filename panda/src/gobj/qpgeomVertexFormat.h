// Filename: qpgeomVertexFormat.h
// Created by:  drose (07Mar05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef qpGEOMVERTEXFORMAT_H
#define qpGEOMVERTEXFORMAT_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "qpgeomVertexAnimationSpec.h"
#include "qpgeomVertexArrayFormat.h"
#include "internalName.h"
#include "luse.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pset.h"
#include "pvector.h"
#include "pta_float.h"
#include "indirectCompareTo.h"
#include "pmutex.h"

class FactoryParams;
class qpGeomVertexData;
class qpGeomMunger;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomVertexFormat
// Description : This class defines the layout of the vertex data
//               arrays stored within a Geom.  The data within a Geom
//               is typically stored within a single interleaved
//               array, but it may also be distributed among multiple
//               different arrays.  Each array is described by a
//               qpGeomVertexArrayFormat object, referenced within
//               this object.
//
//               There are a number of standard pre-defined
//               GeomVertexFormat objects, or you may define your own
//               as needed.  You may also record any combination of
//               standard and/or user-defined data types in your
//               custom GeomVertexFormat constructions.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomVertexFormat : public TypedWritableReferenceCount {
PUBLISHED:
  qpGeomVertexFormat();
  qpGeomVertexFormat(qpGeomVertexArrayFormat *array_format);
  qpGeomVertexFormat(const qpGeomVertexFormat &copy);
  void operator = (const qpGeomVertexFormat &copy);
  virtual ~qpGeomVertexFormat();

  INLINE bool is_registered() const;
  INLINE static CPT(qpGeomVertexFormat) register_format(qpGeomVertexFormat *format);
  INLINE static CPT(qpGeomVertexFormat) register_format(qpGeomVertexArrayFormat *format);

  INLINE const qpGeomVertexAnimationSpec &get_animation() const;
  INLINE void set_animation(const qpGeomVertexAnimationSpec &animation);

  INLINE int get_num_arrays() const;
  INLINE const qpGeomVertexArrayFormat *get_array(int array) const;
  qpGeomVertexArrayFormat *modify_array(int array);
  void set_array(int array, qpGeomVertexArrayFormat *format);
  void remove_array(int array);
  int add_array(qpGeomVertexArrayFormat *array_format);
  void insert_array(int array, qpGeomVertexArrayFormat *array_format);
  void clear_arrays();

  int get_num_data_types() const;
  int get_array_with(int i) const;
  const qpGeomVertexDataType *get_data_type(int i) const;

  int get_array_with(const InternalName *name) const;
  const qpGeomVertexDataType *get_data_type(const InternalName *name) const;
  INLINE bool has_data_type(const InternalName *name) const;

  void remove_data_type(const InternalName *name);

  INLINE int get_num_morphs() const;
  INLINE const InternalName *get_morph_slider(int n) const;
  INLINE const InternalName *get_morph_base(int n) const;
  INLINE const InternalName *get_morph_delta(int n) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;
  void write_with_data(ostream &out, int indent_level, 
                       const qpGeomVertexData *data) const;

  // Some standard vertex formats.  No particular requirement to use
  // one of these, but the DirectX renderers can use these formats
  // directly, whereas any other format will have to be converted
  // first.
  INLINE static const qpGeomVertexFormat *get_v3();
  INLINE static const qpGeomVertexFormat *get_v3n3();
  INLINE static const qpGeomVertexFormat *get_v3t2();
  INLINE static const qpGeomVertexFormat *get_v3n3t2();

  // These formats, with the DirectX-style packed color, are not
  // supported directly by OpenGL.  If you use them, the
  // GLGraphicsStateGuardian will automatically convert to OpenGL
  // form (with a small runtime overhead).
  INLINE static const qpGeomVertexFormat *get_v3cp();
  INLINE static const qpGeomVertexFormat *get_v3cpt2();
  INLINE static const qpGeomVertexFormat *get_v3n3cp();
  INLINE static const qpGeomVertexFormat *get_v3n3cpt2();

  // These formats, with an OpenGL-style four-byte color, are not
  // supported directly by DirectX.  If you use them, the
  // DXGraphicsStateGuardian will automatically convert to DirectX
  // form (with a larger runtime overhead, since DirectX8, and old
  // DirectX9 drivers, require everything to be interleaved together).
  INLINE static const qpGeomVertexFormat *get_v3c4();
  INLINE static const qpGeomVertexFormat *get_v3c4t2();
  INLINE static const qpGeomVertexFormat *get_v3n3c4();
  INLINE static const qpGeomVertexFormat *get_v3n3c4t2();

public:
  bool get_array_info(const InternalName *name,
                      int &array_index,
                      const qpGeomVertexDataType *&data_type) const;

  int compare_to(const qpGeomVertexFormat &other) const;

private:
  class Registry;
  INLINE static Registry *get_registry();
  static void make_registry();

  void do_register();
  void do_unregister();

  bool _is_registered;

  qpGeomVertexAnimationSpec _animation;

  typedef pvector< PT(qpGeomVertexArrayFormat) > Arrays;
  Arrays _arrays;

  class DataTypeRecord {
  public:
    int _array_index;
    int _data_type_index;
  };

  typedef pmap<const InternalName *, DataTypeRecord> DataTypesByName;
  DataTypesByName _data_types_by_name;

  class MorphRecord {
  public:
    CPT(InternalName) _slider;
    CPT(InternalName) _base;
    CPT(InternalName) _delta;
  };
  typedef pvector<MorphRecord> Morphs;
  Morphs _morphs;

  // This is the global registry of all currently-in-use formats.
  typedef pset<qpGeomVertexFormat *, IndirectCompareTo<qpGeomVertexFormat> > Formats;
  class EXPCL_PANDA Registry {
  public:
    Registry();
    CPT(qpGeomVertexFormat) register_format(qpGeomVertexFormat *format);
    INLINE CPT(qpGeomVertexFormat) register_format(qpGeomVertexArrayFormat *format);
    void unregister_format(qpGeomVertexFormat *format);

    Formats _formats;

    CPT(qpGeomVertexFormat) _v3;
    CPT(qpGeomVertexFormat) _v3n3;
    CPT(qpGeomVertexFormat) _v3t2;
    CPT(qpGeomVertexFormat) _v3n3t2;
    
    CPT(qpGeomVertexFormat) _v3cp;
    CPT(qpGeomVertexFormat) _v3n3cp;
    CPT(qpGeomVertexFormat) _v3cpt2;
    CPT(qpGeomVertexFormat) _v3n3cpt2;
    
    CPT(qpGeomVertexFormat) _v3c4;
    CPT(qpGeomVertexFormat) _v3n3c4;
    CPT(qpGeomVertexFormat) _v3c4t2;
    CPT(qpGeomVertexFormat) _v3n3c4t2;
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
    register_type(_type_handle, "qpGeomVertexFormat",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeomVertexFormat::Registry;
  friend class qpGeomMunger;
};

INLINE ostream &operator << (ostream &out, const qpGeomVertexFormat &obj);

#include "qpgeomVertexFormat.I"

#endif
