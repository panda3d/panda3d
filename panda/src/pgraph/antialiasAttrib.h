// Filename: antialiasAttrib.h
// Created by:  drose (26Jan05)
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

#ifndef ANTIALIASATTRIB_H
#define ANTIALIASATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : AntialiasAttrib
// Description : Specifies whether or how to enable antialiasing, if
//               supported by the backend renderer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA AntialiasAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_none        = 0x0000,
    M_point       = 0x0001,
    M_line        = 0x0002,
    M_polygon     = 0x0004,
    M_multisample = 0x0008,
    M_auto        = 0x001f,
    M_type_mask   = 0x001f,

    // Extra add-on bits for performance/quality hints.
    M_faster      = 0x0020,
    M_better      = 0x0040,
    M_dont_care   = 0x0060,
  };

private:
  INLINE AntialiasAttrib(unsigned short mode);

PUBLISHED:
  static CPT(RenderAttrib) make(unsigned short mode);

  INLINE unsigned short get_mode() const;
  INLINE unsigned short get_mode_type() const;
  INLINE unsigned short get_mode_quality() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  unsigned short _mode;

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
    register_type(_type_handle, "AntialiasAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "antialiasAttrib.I"

#endif

