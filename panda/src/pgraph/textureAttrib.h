// Filename: textureAttrib.h
// Created by:  drose (21Feb02)
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

#ifndef TEXTUREATTRIB_H
#define TEXTUREATTRIB_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "texture.h"
#include "textureStage.h"
#include "updateSeq.h"
#include "indirectLess.h"
#include "geom.h"
#include "ordered_vector.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureAttrib
// Description : Indicates which texture should be applied as the
//               primary texture.  Also see TextureAttrib2 for the
//               secondary texture.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureAttrib : public RenderAttrib {
private:
  INLINE TextureAttrib();
  INLINE TextureAttrib(const TextureAttrib &copy);

PUBLISHED:
  // These methods are deprecated, but they remain for now, for
  // backward compatibility.  They treat the TextureAttrib as a
  // single-texture application.
  static CPT(RenderAttrib) make(Texture *tex);
  static CPT(RenderAttrib) make_off();

  INLINE bool is_off() const;
  INLINE Texture *get_texture() const;

  // The following methods define the new multitexture mode for
  // TextureAttrib.  Each TextureAttrib can add or remove individual
  // texture stages from the complete set of textures that are to be
  // applied; this is similar to the mechanism of LightAttrib.
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make_all_off();

  INLINE int get_num_on_stages() const;
  INLINE TextureStage *get_on_stage(int n) const;
  INLINE bool has_on_stage(TextureStage *stage) const;
  INLINE Texture *get_on_texture(TextureStage *stage) const;

  INLINE int get_num_off_stages() const;
  INLINE TextureStage *get_off_stage(int n) const;
  INLINE bool has_off_stage(TextureStage *stage) const;
  INLINE bool has_all_off() const;

  INLINE bool is_identity() const;

  CPT(RenderAttrib) add_on_stage(TextureStage *stage, Texture *tex) const;
  CPT(RenderAttrib) remove_on_stage(TextureStage *stage) const;
  CPT(RenderAttrib) add_off_stage(TextureStage *stage) const;
  CPT(RenderAttrib) remove_off_stage(TextureStage *stage) const;
  CPT(RenderAttrib) unify_texture_stages(TextureStage *stage) const;

public:
  INLINE const Geom::ActiveTextureStages &get_on_stages() const;
  CPT(TextureAttrib) filter_to_max(int max_texture_stages) const;

  virtual void issue(GraphicsStateGuardianBase *gsg) const;
  virtual void output(ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;
  virtual RenderAttrib *make_default_impl() const;

private:
  INLINE void check_sorted() const;
  void sort_on_stages();

private:
  typedef Geom::ActiveTextureStages OnStages;
  OnStages _on_stages;

  typedef ov_set<TextureStage *> OffStages;
  OffStages _off_stages;
  bool _off_all_stages;

  typedef pmap< TextureStage *, PT(Texture) > OnTextures;
  OnTextures _on_textures;

  typedef pmap< int, CPT(TextureAttrib) > Filtered;
  Filtered _filtered;

  UpdateSeq _sort_seq;

  int _num_on_textures;  //temporary count to complete_pointers from fill_in

  static CPT(RenderAttrib) _empty_attrib;
  static CPT(RenderAttrib) _all_off_attrib;

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
    register_type(_type_handle, "TextureAttrib",
                  RenderAttrib::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "textureAttrib.I"

#endif

