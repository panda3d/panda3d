// Filename: auxBitplaneAttrib.h
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

#ifndef AUXBITPLANEATTRIB_H
#define AUXBITPLANEATTRIB_H

#include "pandabase.h"
#include "renderAttrib.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : AuxBitplaneAttrib
// Description : Modern frame buffers can have 'aux' bitplanes, which
//               are additional bitplanes above and beyond the
//               standard depth and color.  This attrib controls what
//               gets rendered into those additional bitplanes.  It
//               can also affect what goes into the alpha channel
//               of the primary color buffer.
//
//               ABO_glow: copy the glow map into the alpha channel
//               of the primary frame buffer.  If there is no glow
//               map, set it to zero.  Caveat: it is not
//               possible to write glow or depth values to the
//               framebuffer alpha channel at the same time as using
//               alpha blending or alpha testing.  Any attempt to use
//               transparency, blending, or alpha testing will cause
//               this flag to be overridden.
//
//               ABO_aux_normal: put the camera-space normal into
//               the into the R,G components of the first auxiliary
//               bitplane.
//
//               ABO_aux_modelz: put the clip-space Z coordinate of
//               the center of the model (after perspective divide)
//               into the B channel of the first auxiliary bitplane.
//
//               ABO_aux_glow: put a copy of the glow map into the
//               alpha channel of the first auxiliary bitplane.
//               If there is no glow map, set it to zero.
//
//               AuxBitplaneAttrib is relevant only when shader
//               generation is enabled. Otherwise, it has no effect.
//
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH AuxBitplaneAttrib : public RenderAttrib {
private:
  INLINE AuxBitplaneAttrib(int outputs);

PUBLISHED:
  enum AuxBitplaneOutput {
    ABO_glow = 1,
    
    ABO_aux_normal = 2,
    ABO_aux_glow = 4,
  };
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make(int outputs);
  
  INLINE int get_outputs() const;
  
public:
  virtual void output(ostream &out) const;
  virtual void store_into_slot(AttribSlots *slots) const;
  
protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;
  
private:
  int _outputs;

  static CPT(RenderAttrib) _default;

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
    register_type(_type_handle, "AuxBitplaneAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "auxBitplaneAttrib.I"

#endif

