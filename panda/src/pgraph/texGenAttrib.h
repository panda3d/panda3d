// Filename: texGenAttrib.h
// Created by:  masad (21Jun04)
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

#ifndef TEXGENATTRIB_H
#define TEXGENATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "texture.h"

////////////////////////////////////////////////////////////////////
//       Class : TexGenAttrib
// Description : Calculates new texture coordinates for reflection 
//               and refraction maps. This attrib is used to get a
//               water like surface.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexGenAttrib : public RenderAttrib {
private:
PUBLISHED:
  enum Mode {
    M_spherical,
    M_cubic,
    M_nothing
  };

private:
  INLINE TexGenAttrib(Mode mode = M_nothing);

PUBLISHED:
  static CPT(RenderAttrib) make(Mode mode);
  static CPT(RenderAttrib) make_off();

  INLINE bool is_off() const;
  INLINE Mode get_mode() const;
  INLINE Texture *get_texture() const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  Mode _mode;
  PT(Texture) _texture;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  //virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "TexGenAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "texGenAttrib.I"

#endif

