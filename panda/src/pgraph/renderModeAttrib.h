// Filename: renderModeAttrib.h
// Created by:  drose (14Mar02)
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

#ifndef RENDERMODEATTRIB_H
#define RENDERMODEATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : RenderModeAttrib
// Description : Specifies how polygons are to be drawn.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderModeAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_filled,
    M_wireframe,
    // M_point

    // Perhaps others to be added later.
  };

private:
  INLINE RenderModeAttrib(Mode mode, float line_width);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode, float line_width = 1.0f);

  INLINE Mode get_mode() const;
  INLINE float get_line_width() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Mode _mode;
  float _line_width;

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
    register_type(_type_handle, "RenderModeAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "renderModeAttrib.I"

#endif

