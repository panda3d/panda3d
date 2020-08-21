/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file logicOpAttrib.I
 * @author rdb
 * @date 2016-03-24
 */

#ifndef LOGICOPATTRIB_H
#define LOGICOPATTRIB_H

#include "pandabase.h"
#include "luse.h"
#include "renderAttrib.h"

class FactoryParams;

/**
 * If enabled, specifies that a custom logical operation be performed instead
 * of any color blending.  Setting it to a value other than M_none will cause
 * color blending to be disabled and the given logic operation to be performed.
 *
 * @since 1.10.0
 */
class EXPCL_PANDA_PGRAPH LogicOpAttrib : public RenderAttrib {
PUBLISHED:
  enum Operation {
    O_none,  // LogicOp disabled, regular blending occurs.
    O_clear, // Clears framebuffer value.
    O_and,
    O_and_reverse,
    O_copy,  // Writes the incoming color to the framebuffer.
    O_and_inverted,
    O_noop,  // Leaves the framebuffer value unaltered.
    O_xor,
    O_or,
    O_nor,
    O_equivalent,
    O_invert,
    O_or_reverse,
    O_copy_inverted,
    O_or_inverted,
    O_nand,
    O_set,   // Sets all the bits in the framebuffer to 1.
  };

private:
  INLINE LogicOpAttrib(Operation op);

PUBLISHED:
  static CPT(RenderAttrib) make_off();
  static CPT(RenderAttrib) make(Operation op);
  static CPT(RenderAttrib) make_default();

  INLINE Operation get_operation() const;
  MAKE_PROPERTY(operation, get_operation);

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;

private:
  Operation _op;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

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
    register_type(_type_handle, "LogicOpAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new LogicOpAttrib(O_none));
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

EXPCL_PANDA_PGRAPH std::ostream &operator << (std::ostream &out, LogicOpAttrib::Operation op);

#include "logicOpAttrib.I"

#endif
