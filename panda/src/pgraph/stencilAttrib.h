// Filename: stencilAttrib.h
// Created by:  aignacio (18May06)
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

#ifndef STENCILATTRIB_H
#define STENCILATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : StencilAttrib
// Description : A StencilAttrib is a collection of all stencil render
//               states.  The render states in a StencilAttrib are 
//               read-only.  A StencilAttrib is created with make or 
//               make_2_sided.  To determine if two sided stencil is 
//               supported, call the function GraphicsStateGuardian::
//               get_supports_two_sided_stencil.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH StencilAttrib : public RenderAttrib {

private:
  StencilAttrib();

PUBLISHED:

  // enums are duplicated here from class StencilRenderStates for use in Python
  enum StencilRenderState
  {
    SRS_front_enable,
    SRS_back_enable,

    SRS_front_comparison_function,
    SRS_front_stencil_fail_operation,
    SRS_front_stencil_pass_z_fail_operation,
    SRS_front_stencil_pass_z_pass_operation,

    SRS_reference,
    SRS_read_mask,
    SRS_write_mask,

    SRS_back_comparison_function,
    SRS_back_stencil_fail_operation,
    SRS_back_stencil_pass_z_fail_operation,
    SRS_back_stencil_pass_z_pass_operation,

    SRS_clear,
    SRS_clear_value,
    
    SRS_total,

    SRS_first = 0,
  };

  enum StencilComparisonFunction
  {
    SCF_never,
    SCF_less_than,
    SCF_equal,
    SCF_less_than_or_equal,
    SCF_greater_than,
    SCF_not_equal,
    SCF_greater_than_or_equal,
    SCF_always,
  };

  enum StencilOperation
  {
    SO_keep,
    SO_zero,
    SO_replace,
    SO_increment,
    SO_decrement,
    SO_invert,
    SO_increment_saturate,
    SO_decrement_saturate,
  };

  enum StencilMask
  {
    SM_default = ~0,
  };

  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make_default();

  static CPT(RenderAttrib) make(
    unsigned int front_enable,
    unsigned int front_comparison_function,
    unsigned int stencil_fail_operation,
    unsigned int stencil_pass_z_fail_operation,
    unsigned int front_stencil_pass_z_pass_operation,
    unsigned int reference,
    unsigned int read_mask,
    unsigned int write_mask);

  static CPT(RenderAttrib) make_2_sided(
    unsigned int front_enable,
    unsigned int back_enable,
    unsigned int front_comparison_function,
    unsigned int stencil_fail_operation,
    unsigned int stencil_pass_z_fail_operation,
    unsigned int front_stencil_pass_z_pass_operation,
    unsigned int reference,
    unsigned int read_mask,
    unsigned int write_mask,
    unsigned int back_comparison_function,
    unsigned int back_stencil_fail_operation,
    unsigned int back_stencil_pass_z_fail_operation,
    unsigned int back_stencil_pass_z_pass_operation);

  static CPT(RenderAttrib) make_with_clear(
    unsigned int front_enable,
    unsigned int front_comparison_function,
    unsigned int stencil_fail_operation,
    unsigned int stencil_pass_z_fail_operation,
    unsigned int front_stencil_pass_z_pass_operation,
    unsigned int reference,
    unsigned int read_mask,
    unsigned int write_mask,
    unsigned int clear,
    unsigned int clear_value);

  static CPT(RenderAttrib) make_2_sided_with_clear(
    unsigned int front_enable,
    unsigned int back_enable,
    unsigned int front_comparison_function,
    unsigned int stencil_fail_operation,
    unsigned int stencil_pass_z_fail_operation,
    unsigned int front_stencil_pass_z_pass_operation,
    unsigned int reference,
    unsigned int read_mask,
    unsigned int write_mask,
    unsigned int back_comparison_function,
    unsigned int back_stencil_fail_operation,
    unsigned int back_stencil_pass_z_fail_operation,
    unsigned int back_stencil_pass_z_pass_operation,
    unsigned int clear,
    unsigned int clear_value);

  INLINE unsigned int get_render_state (unsigned int render_state_identifier) const;

public:
  static const char *stencil_render_state_name_array [SRS_total];
  
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  unsigned int _stencil_render_states [SRS_total];

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }

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
    register_type(_type_handle, "StencilAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, make_default);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "stencilAttrib.I"

#endif

