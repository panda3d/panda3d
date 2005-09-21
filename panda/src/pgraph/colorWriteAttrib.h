// Filename: colorWriteAttrib.h
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

#ifndef COLORWRITEATTRIB_H
#define COLORWRITEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : ColorWriteAttrib
// Description : Enables or disables writing to the color buffer.
//               This is primarily useful for certain special effects
//               in which it is important to write to the depth buffer
//               without affecting the color buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorWriteAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_off,
    M_on
  };

private:
  INLINE ColorWriteAttrib(Mode mode = M_on);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);

  INLINE Mode get_mode() const;

public:
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Mode _mode;

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
    register_type(_type_handle, "ColorWriteAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colorWriteAttrib.I"

#endif

