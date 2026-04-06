/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file instanceList.h
 * @author rdb
 * @date 2019-03-10
 */

#ifndef INSTANCELIST_H
#define INSTANCELIST_H

#include "pandabase.h"
#include "copyOnWriteObject.h"
#include "transformState.h"
#include "pvector.h"
#include "geomVertexArrayData.h"

class BitArray;
class FactoryParams;

/**
 * This structure stores a list of per-instance data, used by InstancedNode.
 *
 * @since 1.11.0
 */
class EXPCL_PANDA_PGRAPH InstanceList : public CopyOnWriteObject {
protected:
  virtual PT(CopyOnWriteObject) make_cow_copy() override;

PUBLISHED:
  InstanceList();
  InstanceList(const InstanceList &copy);
  virtual ~InstanceList();
  ALLOC_DELETED_CHAIN(InstanceList);

  /**
   * An individual instance in an InstanceList.
   *
   * @since 1.11.0
   */
  class EXPCL_PANDA_PGRAPH Instance {
  public:
    INLINE explicit Instance();
    INLINE explicit Instance(CPT(TransformState) transform);

  PUBLISHED:
    INLINE LPoint3 get_pos() const;
    INLINE void set_pos(const LPoint3 &);
    INLINE void set_pos(PN_stdfloat x, PN_stdfloat y, PN_stdfloat z);

    INLINE LVecBase3 get_hpr() const;
    INLINE void set_hpr(const LVecBase3 &);
    INLINE void set_hpr(PN_stdfloat h, PN_stdfloat p, PN_stdfloat r);

    INLINE LQuaternion get_quat() const;
    INLINE void set_quat(const LQuaternion &);

    INLINE LVecBase3 get_scale() const;
    INLINE void set_scale(const LVecBase3 &);
    INLINE void set_scale(PN_stdfloat sx, PN_stdfloat sy, PN_stdfloat sz);

    INLINE const LMatrix4 &get_mat() const;
    INLINE void set_mat(const LMatrix4 &mat);

    INLINE const TransformState *get_transform() const;
    INLINE void set_transform(CPT(TransformState));
    MAKE_PROPERTY(transform, get_transform);

  private:
    CPT(TransformState) _transform;
  };

  void append(Instance instance);
  void append(const TransformState *transform);
  void append(const LPoint3 &pos,
              const LVecBase3 &hpr = LVecBase3(0),
              const LVecBase3 &scale = LVecBase3(1));
  void append(const LPoint3 &pos,
              const LQuaternion &quat,
              const LVecBase3 &scale = LVecBase3(1));

  INLINE size_t size() const;
  INLINE const Instance &operator [] (size_t n) const;
  INLINE Instance &operator [] (size_t n);
  INLINE void clear();
  INLINE void reserve(size_t);

  void xform(const LMatrix4 &mat);

public:
  typedef pvector<Instance> Instances;
  typedef Instances::iterator iterator;
  typedef Instances::const_iterator const_iterator;

  INLINE bool empty() const;

  INLINE iterator begin();
  INLINE const_iterator begin() const;
  INLINE const_iterator cbegin() const;

  INLINE iterator end();
  INLINE const_iterator end() const;
  INLINE const_iterator cend() const;

  CPT(InstanceList) without(const BitArray &mask) const;

  CPT(GeomVertexArrayData) get_array_data(const GeomVertexArrayFormat *format) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

private:
  Instances _instances;

  mutable CPT(GeomVertexArrayData) _cached_array;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg) override;
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager) override;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager) override;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CopyOnWriteObject::init_type();
    register_type(_type_handle, "InstanceList",
                  CopyOnWriteObject::get_class_type());
  }
  virtual TypeHandle get_type() const override {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() override {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

inline std::ostream &operator <<(std::ostream &out, const InstanceList &list) {
  list.output(out);
  return out;
}

#include "instanceList.I"

#endif
