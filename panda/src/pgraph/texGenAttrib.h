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

#include "geom.h"
#include "renderAttrib.h"
#include "textureStage.h"
#include "texture.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : TexGenAttrib
// Description : Computes texture coordinates for geometry
//               automatically based on vertex position and/or normal.
//               This can be used to implement reflection and/or
//               refraction maps, for instance to make shiny surfaces,
//               as well as other special effects such as projective
//               texturing.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexGenAttrib : public RenderAttrib {
PUBLISHED:
  enum Mode {
    M_off,
    M_sphere_map,
    M_cube_map,
    M_world_position,
    M_object_position,
    M_eye_position,
    M_object_normal,
  };

protected:
  INLINE TexGenAttrib();
  INLINE TexGenAttrib(const TexGenAttrib &copy);

public:
  virtual ~TexGenAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make();

  CPT(RenderAttrib) add_stage(TextureStage *stage, Mode mode) const;
  CPT(RenderAttrib) remove_stage(TextureStage *stage) const;

  bool is_empty() const;
  bool has_stage(TextureStage *stage) const;
  Mode get_mode(TextureStage *stage) const;

public:
  INLINE const Geom::NoTexCoordStages &get_no_texcoords() const;

  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  typedef pmap<PT(TextureStage), Mode> Stages;
  Stages _stages;

  // This is a set of TextureStage pointers for which texture
  // coordinates will not be needed from the Geom.  It's redundant;
  // it's almost the same set that is listed in _stages, above.  It's
  // just here as an optimization to pass to
  // Geom::setup_multitexcoord_iterator() during rendering.
  Geom::NoTexCoordStages _no_texcoords;

  // This element is only used during reading from a bam file.  It has
  // no meaningful value any other time.
  pvector<Mode> _read_modes;

  static CPT(RenderAttrib) _empty_attrib;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

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

