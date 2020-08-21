/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lightAttrib.h
 * @author drose
 * @date 2002-03-26
 */

#ifndef LIGHTATTRIB_H
#define LIGHTATTRIB_H

#include "pandabase.h"

#include "light.h"
#include "renderAttrib.h"
#include "nodePath.h"
#include "ordered_vector.h"
#include "pmap.h"

/**
 * Indicates which set of lights should be considered "on" to illuminate
 * geometry at this level and below.  A LightAttrib can either add lights or
 * remove lights from the total set of "on" lights.
 */
class EXPCL_PANDA_PGRAPH LightAttrib : public RenderAttrib {
protected:
  INLINE LightAttrib();
  LightAttrib(const LightAttrib &copy);

PUBLISHED:
  virtual ~LightAttrib();

  // This is the old, deprecated interface to LightAttrib.  Do not use any of
  // these methods for new code; these methods will be removed soon.
  enum Operation {
    O_set,
    O_add,
    O_remove
  };
  static CPT(RenderAttrib) make(Operation op,
                                Light *light);
  static CPT(RenderAttrib) make(Operation op,
                                Light *light1, Light *light2);
  static CPT(RenderAttrib) make(Operation op,
                                Light *light1, Light *light2,
                                Light *light3);
  static CPT(RenderAttrib) make(Operation op,
                                Light *light1, Light *light2,
                                Light *light3, Light *light4);
  static CPT(RenderAttrib) make_default();

  Operation get_operation() const;

  int get_num_lights() const;
  Light *get_light(int n) const;
  bool has_light(Light *light) const;

  CPT(RenderAttrib) add_light(Light *light) const;
  CPT(RenderAttrib) remove_light(Light *light) const;


  // The following is the new, more general interface to the LightAttrib.
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make_all_off();

  INLINE size_t get_num_on_lights() const;
  INLINE size_t get_num_non_ambient_lights() const;
  INLINE NodePath get_on_light(size_t n) const;
  MAKE_SEQ(get_on_lights, get_num_on_lights, get_on_light);
  INLINE bool has_on_light(const NodePath &light) const;
  INLINE bool has_any_on_light() const;

  INLINE size_t get_num_off_lights() const;
  INLINE NodePath get_off_light(size_t n) const;
  MAKE_SEQ(get_off_lights, get_num_off_lights, get_off_light);
  INLINE bool has_off_light(const NodePath &light) const;
  INLINE bool has_all_off() const;

  INLINE bool is_identity() const;

  CPT(RenderAttrib) add_on_light(const NodePath &light) const;
  CPT(RenderAttrib) remove_on_light(const NodePath &light) const;
  CPT(RenderAttrib) add_off_light(const NodePath &light) const;
  CPT(RenderAttrib) remove_off_light(const NodePath &light) const;

  NodePath get_most_important_light() const;
  LColor get_ambient_contribution() const;

  MAKE_SEQ_PROPERTY(on_lights, get_num_on_lights, get_on_light);
  MAKE_SEQ_PROPERTY(off_lights, get_num_off_lights, get_off_light);

public:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  INLINE void check_sorted() const;
  void sort_on_lights();

private:
  typedef ov_set<NodePath> Lights;
  Lights _on_lights, _off_lights;
  bool _off_all_lights;

  // These are sorted in descending order of priority, with the ambient lights
  // sorted last.
  typedef pvector<NodePath> OrderedLights;
  OrderedLights _sorted_on_lights;
  size_t _num_non_ambient_lights;

  UpdateSeq _sort_seq;

  static CPT(RenderAttrib) _empty_attrib;
  static CPT(RenderAttrib) _all_off_attrib;

PUBLISHED:
  static int get_class_slot() {
    return _attrib_slot;
  }
  virtual int get_slot() const {
    return get_class_slot();
  }
  MAKE_PROPERTY(class_slot, get_class_slot);

public:
  // This data is only needed when reading from a bam file.
  typedef pvector<PT(PandaNode) > NodeList;
  class BamAuxData : public BamReader::AuxData {
  public:
    // We hold a pointer to each of the PandaNodes on the on_list and
    // off_list.  We will later convert these to NodePaths in finalize().
    int _num_off_lights;
    int _num_on_lights;
    NodeList _off_list;
    NodeList _on_list;
  };

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderAttrib::init_type();
    register_type(_type_handle, "LightAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 20, new LightAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "lightAttrib.I"

#endif
