// Filename: transformAttrib.h
// Created by:  drose (23Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef TRANSFORMATTRIB_H
#define TRANSFORMATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "luse.h"

////////////////////////////////////////////////////////////////////
//       Class : TransformAttrib
// Description : Indicates a coordinate-system transform on vertices.
//               TransformAttribs are the primary means for storing
//               transformations on the scene graph.
//
//               Transforms may be specified in one of two ways:
//               componentwise, with a pos-hpr-scale, or with an
//               arbitrary transform matrix.  If you specify a
//               transform componentwise, it will remember its
//               original components.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformAttrib : public RenderAttrib {
private:
  INLINE TransformAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make_identity();
  static CPT(RenderAttrib) make_components(const LVecBase3f &pos,
                                           const LVecBase3f &hpr, 
                                           const LVecBase3f &scale);
  static CPT(RenderAttrib) make_mat(const LMatrix4f &mat);

  INLINE bool is_identity() const;
  INLINE bool is_singular() const;
  INLINE bool has_components() const;
  INLINE const LVecBase3f &get_pos() const;
  INLINE const LVecBase3f &get_hpr() const;
  INLINE const LVecBase3f &get_scale() const;
  INLINE const LMatrix4f &get_mat() const;

public:
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  INLINE void check_singular() const;
  INLINE void check_components() const;
  INLINE void check_mat() const;
  void calc_singular();
  void calc_components();
  void calc_mat();

  enum Flags {
    F_is_identity      =  0x0001,
    F_is_singular      =  0x0002,
    F_singular_known   =  0x0004,
    F_components_given =  0x0008,
    F_components_known =  0x0010,
    F_has_components   =  0x0020,
    F_mat_known        =  0x0040,
  };
  LVecBase3f _pos, _hpr, _scale;
  LMatrix4f _mat;
  
  short _flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "TransformAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "transformAttrib.I"

#endif

