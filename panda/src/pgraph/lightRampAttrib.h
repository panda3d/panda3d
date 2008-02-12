// Filename: lightRampAttrib.h
// Created by:  drose (04Mar02)
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

#ifndef LIGHTRAMPATTRIB_H
#define LIGHTRAMPATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : LightRampAttrib
// Description : The LightRampAttrib alters the light level reaching
//               the surface of the model by applying a "ramp" function.
//               Typically, this is used for cartoon lighting, in which
//               case the ramp is a step-function.  
//
//               LightRampAttrib is relevant only when lighting and
//               shader generation are both enabled. Otherwise, it has
//               no effect.  The light ramp only affects the diffuse
//               contribution.  Ambient light is not ramped.
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH LightRampAttrib : public RenderAttrib {
private:
  INLINE LightRampAttrib();

PUBLISHED:
  enum LightRampMode {
    LRT_identity,
    LRT_single_threshold,
    LRT_double_threshold,
  };
  static CPT(RenderAttrib) make_identity();
  static CPT(RenderAttrib) make_single_threshold(float thresh0, float lev0);
  static CPT(RenderAttrib) make_double_threshold(float thresh0, float lev0, float thresh1, float lev1);
  
  INLINE LightRampMode get_mode() const;
  INLINE float get_level(int n) const;
  INLINE float get_threshold(int n) const;
  
public:
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;
  
protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;
  
private:
  LightRampMode _mode;
  float _level[2];
  float _threshold[2];

  static CPT(RenderAttrib) _identity;

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
    register_type(_type_handle, "LightRampAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lightRampAttrib.I"

#endif

