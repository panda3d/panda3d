// Filename: lightAttrib.h
// Created by:  drose (26Mar02)
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

#ifndef LIGHTATTRIB_H
#define LIGHTATTRIB_H

#include "pandabase.h"

#include "light.h"
#include "renderAttrib.h"
#include "ordered_vector.h"

////////////////////////////////////////////////////////////////////
//       Class : LightAttrib
// Description : Indicates which set of lights should be considered
//               "on" to illuminate geometry at this level and below.
//               A LightAttrib can either add lights or remove lights
//               from the total set of "on" lights.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA LightAttrib : public RenderAttrib {
private:
  INLINE LightAttrib();

PUBLISHED:
  enum Operation {
    O_set,
    O_add,
    O_remove
  };

  static CPT(RenderAttrib) make_all_off();
  static CPT(RenderAttrib) make(Operation op, 
                                Light *light);
  static CPT(RenderAttrib) make(Operation op, 
                                Light *light1, Light *light2);
  static CPT(RenderAttrib) make(Operation op, 
                                Light *light1, Light *light2,
                                Light *light3);
  static CPT(RenderAttrib) make(Operation op, 
                                Light *light1, Light *light2,
                                Light *light3, Light *light4);

  INLINE Operation get_operation() const;

  INLINE int get_num_lights() const;
  INLINE Light *get_light(int n) const;
  bool has_light(Light *light) const;

  INLINE CPT(RenderAttrib) add_light(Light *light) const;
  INLINE CPT(RenderAttrib) remove_light(Light *light) const;

  INLINE bool is_identity() const;
  INLINE bool is_all_off() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  CPT(RenderAttrib) do_add(const LightAttrib *other, Operation op) const;
  CPT(RenderAttrib) do_remove(const LightAttrib *other, Operation op) const;

private:
  Operation _operation;
  typedef ov_set< PT(Light) > Lights;
  Lights _lights;

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
    RenderAttrib::init_type();
    register_type(_type_handle, "LightAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lightAttrib.I"

#endif

