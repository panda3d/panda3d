// Filename: transparencyAttrib.h
// Created by:  drose (28Feb02)
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

#ifndef TRANSPARENCYATTRIB_H
#define TRANSPARENCYATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : TransparencyAttrib
// Description : This controls the enabling of transparency.  Simply
//               setting an alpha component to non-1 does not in
//               itself make an object transparent; you must also
//               enable transparency mode with a suitable
//               TransparencyTransition.  Similarly, it is wasteful to
//               render an object with a TransparencyTransition in
//               effect unless you actually want it to be at least
//               partially transparent (and it has alpha components
//               less than 1).
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransparencyAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_none,             // No transparency in effect.
    M_alpha,            // Writes to depth buffer of transp objects disabled
    M_alpha_sorted,     // Assumes transp objects are depth sorted
    M_multisample,      // Source alpha values modified to 1.0 before writing
    M_multisample_mask, // Source alpha values not modified
    M_binary,           // Only writes pixels with alpha = 1.0
    M_dual,             // 2-pass: draws opaque, then draws transparent
  };

private:
  INLINE TransparencyAttrib(Mode mode = M_none);

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
    register_type(_type_handle, "TransparencyAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "transparencyAttrib.I"

#endif

