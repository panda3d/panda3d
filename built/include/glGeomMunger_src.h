/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file glGeomMunger_src.h
 * @author drose
 * @date 2005-03-10
 */

#include "pandabase.h"
#include "standardMunger.h"
#include "graphicsStateGuardian.h"
#include "textureAttrib.h"
#include "texGenAttrib.h"
#include "renderState.h"
#include "weakPointerTo.h"
#include "weakPointerCallback.h"

class CLP(GeomContext);

/**
 * This specialization on GeomMunger finesses vertices for OpenGL rendering.
 * In particular, it makes sure colors aren't stored in DirectX's packed_argb
 * format.
 */
class EXPCL_GL CLP(GeomMunger) : public StandardMunger, public WeakPointerCallback {
public:
  CLP(GeomMunger)(GraphicsStateGuardian *gsg, const RenderState *state);
  virtual ~CLP(GeomMunger)();
  ALLOC_DELETED_CHAIN_DECL(CLP(GeomMunger));

  virtual void wp_callback(void *);

protected:
  virtual CPT(GeomVertexFormat) munge_format_impl(const GeomVertexFormat *orig,
                                                  const GeomVertexAnimationSpec &animation);
  virtual CPT(GeomVertexFormat) premunge_format_impl(const GeomVertexFormat *orig);

  virtual int compare_to_impl(const GeomMunger *other) const;
  virtual int geom_compare_to_impl(const GeomMunger *other) const;

private:
  WCPT(TextureAttrib) _texture;
  WCPT(TexGenAttrib) _tex_gen;

  typedef pset<CLP(GeomContext) *> GeomContexts;
  GeomContexts _geom_contexts;

  enum Flags {
    F_interleaved_arrays   = 0x0001,
    F_parallel_arrays      = 0x0002,
  };
  int _flags;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    StandardMunger::init_type();
    register_type(_type_handle, CLASSPREFIX_QUOTED "GeomMunger",
                  StandardMunger::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class CLP(GeomContext);
};

#include "glGeomMunger_src.I"
