// Filename: renderState.h
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

#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "cachedTypedWritableReferenceCount.h"
#include "pointerTo.h"
#include "ordered_vector.h"
#include "updateSeq.h"
#include "pStatCollector.h"
#include "renderModeAttrib.h"
#include "texGenAttrib.h"
#include "texMatrixAttrib.h"

class GraphicsStateGuardianBase;
class FogAttrib;
class CullBinAttrib;
class TransparencyAttrib;
class ColorAttrib;
class ColorScaleAttrib;
class TextureAttrib;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : RenderState
// Description : This represents a unique collection of RenderAttrib
//               objects that correspond to a particular renderable
//               state.
//
//               You should not attempt to create or modify a
//               RenderState object directly.  Instead, call one of
//               the make() functions to create one for you.  And
//               instead of modifying a RenderState object, create a
//               new one.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderState : public CachedTypedWritableReferenceCount {
protected:
  RenderState();

private:
  RenderState(const RenderState &copy);
  void operator = (const RenderState &copy);

public:
  virtual ~RenderState();

PUBLISHED:
  bool operator < (const RenderState &other) const;

  INLINE bool is_empty() const;
  INLINE int get_num_attribs() const;
  INLINE const RenderAttrib *get_attrib(int n) const;
  INLINE int get_override(int n) const;

  int find_attrib(TypeHandle type) const;

  static CPT(RenderState) make_empty();
  static CPT(RenderState) make(const RenderAttrib *attrib, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2,
                               const RenderAttrib *attrib3, int override = 0);
  static CPT(RenderState) make(const RenderAttrib *attrib1,
                               const RenderAttrib *attrib2,
                               const RenderAttrib *attrib3,
                               const RenderAttrib *attrib4, int override = 0);
  static CPT(RenderState) make(const RenderAttrib * const *attrib,
                               int num_attribs, int override = 0);

  CPT(RenderState) compose(const RenderState *other) const;
  CPT(RenderState) invert_compose(const RenderState *other) const;

  CPT(RenderState) add_attrib(const RenderAttrib *attrib, int override = 0) const;
  CPT(RenderState) remove_attrib(TypeHandle type) const;

  CPT(RenderState) adjust_all_priorities(int adjustment) const;

  const RenderAttrib *get_attrib(TypeHandle type) const;
  int get_override(TypeHandle type) const;

  int unref() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

  static int get_max_priority();

  static int get_num_states();
  static int get_num_unused_states();
  static int clear_cache();
  static void list_cycles(ostream &out);
  static void list_states(ostream &out);
  static bool validate_states();

PUBLISHED:
  // These methods are intended for use by low-level code, but they're
  // also handy enough to expose to high-level users.
  INLINE int get_draw_order() const;
  INLINE const FogAttrib *get_fog() const;
  INLINE const CullBinAttrib *get_bin() const;
  INLINE const TransparencyAttrib *get_transparency() const;
  INLINE int get_bin_index() const;
  INLINE const ColorAttrib *get_color() const;
  INLINE const ColorScaleAttrib *get_color_scale() const;
  INLINE const TextureAttrib *get_texture() const;
  INLINE const TexGenAttrib *get_tex_gen() const;
  INLINE const TexMatrixAttrib *get_tex_matrix() const;
  INLINE const RenderModeAttrib *get_render_mode() const;

  INLINE int get_geom_rendering(int geom_rendering) const;

public:
  CPT(RenderState) issue_delta_modify(const RenderState *other, 
                                      GraphicsStateGuardianBase *gsg) const;
  CPT(RenderState) issue_delta_set(const RenderState *other, 
                                   GraphicsStateGuardianBase *gsg) const;

  static void bin_removed(int bin_index);

private:
  class CompositionCycleDescEntry {
  public:
    INLINE CompositionCycleDescEntry(const RenderState *obj,
                                     const RenderState *result,
                                     bool inverted);

    const RenderState *_obj;
    const RenderState *_result;
    bool _inverted;
  };
  typedef pvector<CompositionCycleDescEntry> CompositionCycleDesc;

  static CPT(RenderState) return_new(RenderState *state);
  CPT(RenderState) do_compose(const RenderState *other) const;
  CPT(RenderState) do_invert_compose(const RenderState *other) const;
  static bool r_detect_cycles(const RenderState *start_state,
                              const RenderState *current_state,
                              int length, UpdateSeq this_seq,
                              CompositionCycleDesc *cycle_desc);

  void remove_cache_pointers();

  void determine_bin_index();
  void determine_fog();
  void determine_bin();
  void determine_transparency();
  void determine_color();
  void determine_color_scale();
  void determine_texture();
  void determine_tex_gen();
  void determine_tex_matrix();
  void determine_render_mode();

  INLINE void set_destructing();
  INLINE bool is_destructing() const;

private:
  typedef pset<const RenderState *, indirect_less<const RenderState *> > States;
  static States *_states;
  static CPT(RenderState) _empty_state;

  // This iterator records the entry corresponding to this RenderState
  // object in the above global set.  We keep the iterator around so
  // we can remove it when the RenderState destructs.
  States::iterator _saved_entry;

  // This data structure manages the job of caching the composition of
  // two RenderStates.  It's complicated because we have to be sure to
  // remove the entry if *either* of the input RenderStates destructs.
  // To implement this, we always record Composition entries in pairs,
  // one in each of the two involved RenderState objects.
  class Composition {
  public:
    INLINE Composition();
    INLINE Composition(const Composition &copy);

    // _result is reference counted if and only if it is not the same
    // pointer as this.
    const RenderState *_result;
  };

  // The first element of the map is the object we compose with.  This
  // is not reference counted within this map; instead we store a
  // companion pointer in the other object, and remove the references
  // explicitly when either object destructs.
  typedef phash_map<const RenderState *, Composition, pointer_hash> CompositionCache;
  CompositionCache _composition_cache;
  CompositionCache _invert_composition_cache;

  // This is used to mark nodes as we visit them to detect cycles.
  UpdateSeq _cycle_detect;
  static UpdateSeq _last_cycle_detect;

  static PStatCollector _cache_update_pcollector;
  static PStatCollector _state_compose_pcollector;
  static PStatCollector _state_invert_pcollector;

private:
  // This is the actual data within the RenderState: a set of
  // RenderAttribs.
  class Attribute {
  public:
    INLINE Attribute(const RenderAttrib *attrib, int override);
    INLINE Attribute(int override);
    INLINE Attribute(TypeHandle type);
    INLINE Attribute(const Attribute &copy);
    INLINE void operator = (const Attribute &copy);
    INLINE bool operator < (const Attribute &other) const;
    INLINE int compare_to(const Attribute &other) const;

    TypeHandle _type;
    CPT(RenderAttrib) _attrib;
    int _override;
  };
  typedef ov_set<Attribute> Attributes;
  Attributes _attributes;

  // We cache the index to the associated CullBin, if there happens to
  // be a CullBinAttrib in the state.
  int _bin_index;
  int _draw_order;

  // We also cache the pointer to some critical attribs stored in the
  // state, if they exist.
  const FogAttrib *_fog;
  const CullBinAttrib *_bin;
  const TransparencyAttrib *_transparency;
  const ColorAttrib *_color;
  const ColorScaleAttrib *_color_scale;
  const TextureAttrib *_texture;
  const TexGenAttrib *_tex_gen;
  const TexMatrixAttrib *_tex_matrix;
  const RenderModeAttrib *_render_mode;

  enum Flags {
    F_checked_bin_index    = 0x0001,
    F_checked_fog          = 0x0002,
    F_checked_bin          = 0x0004,
    F_checked_transparency = 0x0008,
    F_checked_color        = 0x0010,
    F_checked_color_scale  = 0x0020,
    F_checked_texture      = 0x0040,
    F_checked_tex_gen      = 0x0080,
    F_checked_tex_matrix   = 0x0100,
    F_checked_render_mode  = 0x0200,
    F_is_destructing       = 0x8000,
  };
  unsigned short _flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);
  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CachedTypedWritableReferenceCount::init_type();
    register_type(_type_handle, "RenderState",
                  CachedTypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const RenderState &state) {
  state.output(out);
  return out;
}

#include "renderState.I"

#endif

