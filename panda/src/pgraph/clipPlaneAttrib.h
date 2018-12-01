/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file clipPlaneAttrib.h
 * @author drose
 * @date 2002-07-11
 */

#ifndef CLIPPINGPLANEATTRIB_H
#define CLIPPINGPLANEATTRIB_H

#include "pandabase.h"

#include "planeNode.h"
#include "renderAttrib.h"
#include "nodePath.h"
#include "ordered_vector.h"
#include "pmap.h"

/**
 * This functions similarly to a LightAttrib.  It indicates the set of
 * clipping planes that modify the geometry at this level and below.  A
 * ClipPlaneAttrib can either add planes or remove planes from the total set
 * of clipping planes in effect.
 */
class EXPCL_PANDA_PGRAPH ClipPlaneAttrib : public RenderAttrib {
private:
  INLINE ClipPlaneAttrib();
  INLINE ClipPlaneAttrib(const ClipPlaneAttrib &copy);

PUBLISHED:

  // This is the old, deprecated interface to ClipPlaneAttrib.  Do not use any
  // of these methods for new code; these methods will be removed soon.
  enum Operation {
    O_set,
    O_add,
    O_remove
  };

  static CPT(RenderAttrib) make(Operation op,
                                PlaneNode *plane);
  static CPT(RenderAttrib) make(Operation op,
                                PlaneNode *plane1, PlaneNode *plane2);
  static CPT(RenderAttrib) make(Operation op,
                                PlaneNode *plane1, PlaneNode *plane2,
                                PlaneNode *plane3);
  static CPT(RenderAttrib) make(Operation op,
                                PlaneNode *plane1, PlaneNode *plane2,
                                PlaneNode *plane3, PlaneNode *plane4);
  static CPT(RenderAttrib) make_default();

  Operation get_operation() const;

  int get_num_planes() const;
  PlaneNode *get_plane(int n) const;
  bool has_plane(PlaneNode *plane) const;

  CPT(RenderAttrib) add_plane(PlaneNode *plane) const;
  CPT(RenderAttrib) remove_plane(PlaneNode *plane) const;


  // The following is the new, more general interface to the ClipPlaneAttrib.
  static CPT(RenderAttrib) make();
  static CPT(RenderAttrib) make_all_off();

  INLINE int get_num_on_planes() const;
  INLINE NodePath get_on_plane(int n) const;
  MAKE_SEQ(get_on_planes, get_num_on_planes, get_on_plane);
  INLINE bool has_on_plane(const NodePath &plane) const;

  INLINE int get_num_off_planes() const;
  INLINE NodePath get_off_plane(int n) const;
  MAKE_SEQ(get_off_planes, get_num_off_planes, get_off_plane);
  INLINE bool has_off_plane(const NodePath &plane) const;
  INLINE bool has_all_off() const;

  INLINE bool is_identity() const;

  CPT(RenderAttrib) add_on_plane(const NodePath &plane) const;
  CPT(RenderAttrib) remove_on_plane(const NodePath &plane) const;
  CPT(RenderAttrib) add_off_plane(const NodePath &plane) const;
  CPT(RenderAttrib) remove_off_plane(const NodePath &plane) const;

  CPT(ClipPlaneAttrib) filter_to_max(int max_clip_planes) const;

public:
  CPT(RenderAttrib) compose_off(const RenderAttrib *other) const;
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderAttrib *other) const;
  virtual size_t get_hash_impl() const;
  virtual CPT(RenderAttrib) compose_impl(const RenderAttrib *other) const;
  virtual CPT(RenderAttrib) invert_compose_impl(const RenderAttrib *other) const;

private:
  INLINE void check_filtered() const;
  void sort_on_planes();

private:
  typedef ov_set<NodePath> Planes;
  Planes _on_planes, _off_planes;
  bool _off_all_planes;

  typedef pmap< int, CPT(ClipPlaneAttrib) > Filtered;
  Filtered _filtered;

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
    // off_list.  We will later convert these to NodePaths in
    // finalize().
    int _num_off_planes;
    int _num_on_planes;
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
    register_type(_type_handle, "ClipPlaneAttrib",
                  RenderAttrib::get_class_type());
    _attrib_slot = register_slot(_type_handle, 100, new ClipPlaneAttrib);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
  static int _attrib_slot;
};

#include "clipPlaneAttrib.I"

#endif
