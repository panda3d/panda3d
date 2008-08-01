// Filename: renderState.h
// Created by:  drose (21Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef RENDERSTATE_H
#define RENDERSTATE_H

#include "pandabase.h"

#include "renderAttrib.h"
#include "nodeCachedReferenceCount.h"
#include "pointerTo.h"
#include "ordered_vector.h"
#include "updateSeq.h"
#include "pStatCollector.h"
#include "renderModeAttrib.h"
#include "texMatrixAttrib.h"
#include "geomMunger.h"
#include "weakPointerTo.h"
#include "reMutex.h"
#include "pmutex.h"
#include "deletedChain.h"
#include "simpleHashMap.h"
#include "cacheStats.h"

class GraphicsStateGuardianBase;
class FogAttrib;
class CullBinAttrib;
class TransparencyAttrib;
class ColorAttrib;
class ColorScaleAttrib;
class TextureAttrib;
class TexGenAttrib;
class ClipPlaneAttrib;
class ScissorAttrib;
class ShaderAttrib;
class FactoryParams;
class AudioVolumeAttrib;

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
class EXPCL_PANDA_PGRAPH RenderState : public NodeCachedReferenceCount {
protected:
  RenderState();

private:
  RenderState(const RenderState &copy);
  void operator = (const RenderState &copy);

public:
  virtual ~RenderState();
  ALLOC_DELETED_CHAIN(RenderState);

PUBLISHED:
  bool operator < (const RenderState &other) const;
  size_t get_hash() const;

  INLINE bool is_empty() const;
  INLINE int get_num_attribs() const;
  INLINE const RenderAttrib *get_attrib(int n) const;
  INLINE int get_override(int n) const;

  INLINE bool has_cull_callback() const;
  bool cull_callback(CullTraverser *trav, const CullTraverserData &data) const;

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
  static CPT(RenderState) make(const AttribSlots *slots, int override = 0);
  
  CPT(RenderState) compose(const RenderState *other) const;
  CPT(RenderState) invert_compose(const RenderState *other) const;

  CPT(RenderState) add_attrib(const RenderAttrib *attrib, int override = 0) const;
  CPT(RenderState) set_attrib(const RenderAttrib *attrib) const;
  CPT(RenderState) set_attrib(const RenderAttrib *attrib, int override) const;
  CPT(RenderState) remove_attrib(TypeHandle type) const;

  CPT(RenderState) adjust_all_priorities(int adjustment) const;

  const RenderAttrib *get_attrib(TypeHandle type) const;
  int get_override(TypeHandle type) const;

  bool unref() const;

  INLINE void cache_ref() const;
  INLINE bool cache_unref() const;
  INLINE void node_ref() const;
  INLINE bool node_unref() const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

  static int get_max_priority();

  static int get_num_states();
  static int get_num_unused_states();
  static int clear_cache();
  static void clear_munger_cache();
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
  INLINE const ClipPlaneAttrib *get_clip_plane() const;
  INLINE const ScissorAttrib *get_scissor() const;
  INLINE const ShaderAttrib *get_shader() const;
  INLINE const AudioVolumeAttrib *get_audio_volume() const;
  
  int get_geom_rendering(int geom_rendering) const;

  const ShaderAttrib *get_generated_shader() const;
  
public:
  void store_into_slots(AttribSlots *slots) const;
  
  static void bin_removed(int bin_index);
  
  INLINE static void flush_level();

private:
  INLINE bool do_cache_unref() const;
  INLINE bool do_node_unref() const;

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

  void release_new();
  void remove_cache_pointers();

  void determine_bin_index();
  void determine_fog();
  INLINE void determine_bin();
  void do_determine_bin();
  INLINE void determine_transparency();
  void do_determine_transparency();
  void determine_color();
  void determine_color_scale();
  void determine_texture();
  void determine_tex_gen();
  void determine_tex_matrix();
  void determine_render_mode();
  void determine_clip_plane();
  void determine_scissor();
  void determine_shader();
  void determine_cull_callback();
  void determine_audio_volume();

  INLINE void set_destructing();
  INLINE bool is_destructing() const;

  INLINE void consider_update_pstats(int old_referenced_bits) const;
  static void update_pstats(int old_referenced_bits, int new_referenced_bits);

public:
  static void init_states();

private:
  // This mutex protects _states.  It also protects any modification
  // to the cache, which is encoded in _composition_cache and
  // _invert_composition_cache.
  static ReMutex *_states_lock;
  typedef phash_set<const RenderState *, indirect_less_hash<const RenderState *> > States;
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
  typedef SimpleHashMap<const RenderState *, Composition, pointer_hash> CompositionCache;
  CompositionCache _composition_cache;
  CompositionCache _invert_composition_cache;

  // This is here to provide a quick cache of GSG + RenderState ->
  // GeomMunger for the cull phase.  It is here because it is faster
  // to look up the GSG in the RenderState pointer than vice-versa,
  // since there are likely to be far fewer GSG's than RenderStates.
  // The code to manage this map lives in
  // GraphicsStateGuardian::get_geom_munger().
  typedef pmap<WCPT(GraphicsStateGuardianBase), PT(GeomMunger) > Mungers;
  Mungers _mungers;
  Mungers::const_iterator _last_mi;

  // This is used to mark nodes as we visit them to detect cycles.
  UpdateSeq _cycle_detect;
  static UpdateSeq _last_cycle_detect;

  static PStatCollector _cache_update_pcollector;
  static PStatCollector _state_compose_pcollector;
  static PStatCollector _state_invert_pcollector;

  static PStatCollector _node_counter;
  static PStatCollector _cache_counter;

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

  // If this state contains an "auto" ShaderAttrib, then an explicit
  // ShaderAttrib will be synthesized by the runtime and stored here.
  // I can't declare this as a ShaderAttrib because that would create
  // a circular include-file dependency problem.  Aaargh.
  CPT(RenderAttrib) _generated_shader;
  
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
  const ClipPlaneAttrib *_clip_plane;
  const ScissorAttrib *_scissor;
  const ShaderAttrib *_shader;
  const AudioVolumeAttrib *_audio_volume;
  
  enum Flags {
    F_checked_bin_index     = 0x000001,
    F_checked_fog           = 0x000002,
    F_checked_bin           = 0x000004,
    F_checked_transparency  = 0x000008,
    F_checked_color         = 0x000010,
    F_checked_color_scale   = 0x000020,
    F_checked_texture       = 0x000040,
    F_checked_tex_gen       = 0x000080,
    F_checked_tex_matrix    = 0x000100,
    F_checked_render_mode   = 0x000200,
    F_checked_clip_plane    = 0x000400,
    F_checked_shader        = 0x000800,
    F_checked_cull_callback = 0x001000,
    F_checked_audio_volume  = 0x002000,
    F_has_cull_callback     = 0x004000,
    F_is_destructing        = 0x008000,
    F_checked_scissor       = 0x010000,
  };
  unsigned int _flags;

  // This mutex protects _flags, and all of the above computed values.
  Mutex _lock;

  static CacheStats _cache_stats;

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
    NodeCachedReferenceCount::init_type();
    register_type(_type_handle, "RenderState",
                  NodeCachedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class GraphicsStateGuardian;
};

INLINE ostream &operator << (ostream &out, const RenderState &state) {
  state.output(out);
  return out;
}

#include "renderState.I"

#endif

