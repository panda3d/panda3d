// Filename: alphaTestAttrib.h
// Created by:  drose (04Mar02)
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

#ifndef ALPHATESTATTRIB_H
#define ALPHATESTATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"

////////////////////////////////////////////////////////////////////
//       Class : AlphaTestAttrib
// Description : Enables or disables writing of pixel to framebuffer
//               based on its alpha value relative to a reference alpha value
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AlphaTestAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {      // defined to match D3DCMPFUNC
    M_never=1,          // Never draw.
    M_less,             // incoming < reference_alpha
    M_equal,            // incoming == reference_alpha
    M_less_equal,       // incoming <= reference_alpha
    M_greater,          // incoming > reference_alpha
    M_not_equal,        // incoming != reference_alpha
    M_greater_equal,    // incoming >= reference_alpha
    M_always            // Always draw.  
  };

private:
  INLINE AlphaTestAttrib(Mode mode = M_always,unsigned int reference_alpha = 0xFF);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode,unsigned int reference_alpha);
  INLINE unsigned int get_reference_alpha() const;
  INLINE Mode get_mode() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Mode _mode;
  unsigned int _reference_alpha;

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
    register_type(_type_handle, "AlphaTestAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "alphaTestAttrib.I"

#endif

