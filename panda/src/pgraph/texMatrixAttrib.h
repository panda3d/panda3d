// Filename: texMatrixAttrib.h
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

#ifndef TEXMATRIXATTRIB_H
#define TEXMATRIXATTRIB_H

#include "pandabase.h"

#include "geom.h"
#include "renderAttrib.h"
#include "textureStage.h"
#include "transformState.h"
#include "pointerTo.h"

class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : TexMatrixAttrib
// Description : Applies a transform matrix to UV's before they are
//               rendered.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TexMatrixAttrib : public RenderAttrib {
protected:
  INLINE TexMatrixAttrib();
  INLINE TexMatrixAttrib(const TexMatrixAttrib &copy);

public:
  virtual ~TexMatrixAttrib();

PUBLISHED:
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make(const LMatrix4f &mat);
  static CPT(RenderAttrib) make(TextureStage *stage, const TransformState *transform);

  CPT(RenderAttrib) add_stage(TextureStage *stage, const TransformState *transform) const;
  CPT(RenderAttrib) remove_stage(TextureStage *stage) const;

  bool is_empty() const;
  bool has_stage(TextureStage *stage) const;

  int get_num_stages() const;
  TextureStage *get_stage(int n) const;

  const LMatrix4f &get_mat() const;
  const LMatrix4f &get_mat(TextureStage *stage) const;

  CPT(TransformState) get_transform(TextureStage *stage) const;

  INLINE int get_geom_rendering(int geom_rendering) const;

public:
  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  INLINE void check_stage_list() const;
  void rebuild_stage_list();

  typedef pmap< PT(TextureStage), CPT(TransformState) > Stages;
  Stages _stages;

  typedef pvector<TextureStage *> StageList;
  StageList _stage_list;
  bool _stage_list_stale;

  // This element is only used during reading from a bam file.  It has
  // no meaningful value any other time.
  size_t _num_stages;

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
    register_type(_type_handle, "TexMatrixAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "texMatrixAttrib.I"

#endif

