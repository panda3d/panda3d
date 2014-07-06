// Filename: internalName.h
// Created by:  drose (15Jul04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef INTERNALNAME_H
#define INTERNALNAME_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "pmap.h"
#include "lightMutex.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : InternalName
// Description : Encodes a string name in a hash table, mapping it to
//               a pointer.  This is used to tokenify names so they
//               may be used efficiently in low-level Panda
//               structures, for instance to differentiate the
//               multiple sets of texture coordinates that might be
//               stored on a Geom.
//
//               InternalNames are hierarchical, with the '.' used by
//               convention as a separator character.  You can
//               construct a single InternalName as a composition of
//               one or more other names, or by giving it a source
//               string directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GOBJ InternalName : public TypedWritableReferenceCount {
private:
  InternalName(InternalName *parent, const string &basename);

PUBLISHED:
  virtual ~InternalName();
  virtual bool unref() const;

  INLINE static PT(InternalName) make(const string &name);
  static PT(InternalName) make(const string &name, int index);
  PT(InternalName) append(const string &basename);

  INLINE InternalName *get_parent() const;
  string get_name() const;
  string join(const string &sep) const;
  INLINE const string &get_basename() const;

  int find_ancestor(const string &basename) const;
  const InternalName *get_ancestor(int n) const;
  const InternalName *get_top() const;
  string get_net_basename(int n) const;

  void output(ostream &out) const;

  // Some predefined built-in names.
  INLINE static PT(InternalName) get_root();
  INLINE static PT(InternalName) get_error();
  INLINE static PT(InternalName) get_vertex();
  INLINE static PT(InternalName) get_normal();
  INLINE static PT(InternalName) get_tangent();
  INLINE static PT(InternalName) get_tangent_name(const string &name);
  INLINE static PT(InternalName) get_binormal();
  INLINE static PT(InternalName) get_binormal_name(const string &name);
  INLINE static PT(InternalName) get_texcoord();
  INLINE static PT(InternalName) get_texcoord_name(const string &name);
  INLINE static PT(InternalName) get_color();
  INLINE static PT(InternalName) get_rotate();
  INLINE static PT(InternalName) get_size();
  INLINE static PT(InternalName) get_aspect_ratio();
  INLINE static PT(InternalName) get_transform_blend();
  INLINE static PT(InternalName) get_transform_weight();
  INLINE static PT(InternalName) get_transform_index();
  INLINE static PT(InternalName) get_morph(InternalName *column, const string &slider);
  INLINE static PT(InternalName) get_index();
  INLINE static PT(InternalName) get_world();
  INLINE static PT(InternalName) get_camera();
  INLINE static PT(InternalName) get_model();
  INLINE static PT(InternalName) get_view();

private:
  PT(InternalName) _parent;
  string _basename;

  typedef phash_map<string, InternalName *, string_hash> NameTable;
  NameTable _name_table;
  LightMutex _name_table_lock;

  static PT(InternalName) _root;
  static PT(InternalName) _error;
  static PT(InternalName) _default;
  static PT(InternalName) _vertex;
  static PT(InternalName) _normal;
  static PT(InternalName) _tangent;
  static PT(InternalName) _binormal;
  static PT(InternalName) _texcoord;
  static PT(InternalName) _color;
  static PT(InternalName) _rotate;
  static PT(InternalName) _size;
  static PT(InternalName) _aspect_ratio;
  static PT(InternalName) _transform_blend;
  static PT(InternalName) _transform_weight;
  static PT(InternalName) _transform_index;
  static PT(InternalName) _index;
  static PT(InternalName) _world;
  static PT(InternalName) _camera;
  static PT(InternalName) _model;
  static PT(InternalName) _view;
  
public:
  // Datagram stuff
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &me);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  static TypedWritable *make_texcoord_from_bam(const FactoryParams &params);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "InternalName",
                  TypedWritableReferenceCount::get_class_type());
    // The _texcoord_type_handle is defined only to support older bam
    // files, generated before we renamed the type to InternalName.
    register_type(_texcoord_type_handle, "TexCoordName",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static TypeHandle _texcoord_type_handle;
};

INLINE ostream &operator << (ostream &out, const InternalName &tcn);

#include "internalName.I"

#endif


  
