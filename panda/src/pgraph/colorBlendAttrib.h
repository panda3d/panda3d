// Filename: colorBlendAttrib.h
// Created by:  drose (29Mar02)
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

#ifndef COLORBLENDATTRIB_H
#define COLORBLENDATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : ColorBlendAttrib
// Description : This specifies how colors are blended into the frame
//               buffer, for special effects.  This overrides
//               transparency if transparency is also specified.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA ColorBlendAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_none,             // Blending is disabled
    M_multiply,         // color already in fbuffer * incoming color
    M_add,              // color already in fbuffer + incoming color
    M_multiply_add,     // color already in fbuffer * incoming color +
                        //   color already in fbuffer
  };

private:
  INLINE ColorBlendAttrib(Mode mode = M_none);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);

  INLINE Mode get_mode() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

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
    register_type(_type_handle, "ColorBlendAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "colorBlendAttrib.I"

#endif

