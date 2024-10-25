/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file transform.h
 * @author rdb
 * @date 2024-10-17
 */

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "pandabase.h"
#include "memoryBase.h"
#include "luse.h"
#include "transformState.h"

class TransformState;

/**
 * Stores an affine coordinate-system transformation.  Transform objects are
 * the primary means for storing transformations on the scene graph.
 *
 * Note that not any matrix can be represented as a Transform; this must be an
 * invertible, non-projective transformation.
 *
 * @since 1.11.0
 */
class EXPCL_PANDA_PGRAPH Transform : public MemoryBase {
PUBLISHED:
  Transform() = default;
  Transform(const TransformState *state) ;
  Transform(const Transform &copy) = default;

  INLINE Transform &operator = (const Transform &other) = default;

  INLINE bool operator == (const Transform &other) const;
  INLINE bool operator != (const Transform &other) const;

  INLINE int compare_to(const Transform &other) const;

  INLINE static Transform make_identity();
  INLINE static Transform make_mat(const LMatrix4 &mat);
  INLINE static Transform make_pos(const LVecBase3 &pos);
  INLINE static Transform make_hpr(const LVecBase3 &hpr);
  INLINE static Transform make_quat(const LQuaternion &quat);
  INLINE static Transform make_scale(const LVecBase3 &scale);
  INLINE static Transform make_shear(const LVecBase3 &shear);
  INLINE static Transform make_pos_hpr(const LVecBase3 &pos,
                                       const LVecBase3 &hpr);
  INLINE static Transform make_pos_hpr_scale(const LVecBase3 &pos,
                                             const LVecBase3 &hpr,
                                             LVecBase3 scale);
  static Transform make_pos_hpr_scale_shear(const LVecBase3 &pos,
                                            const LVecBase3 &hpr,
                                            LVecBase3 scale,
                                            const LVecBase3 &shear);
  INLINE static Transform make_pos_quat(const LVecBase3 &pos,
                                        const LQuaternion &quat);
  static Transform make_pos_quat_scale(const LVecBase3 &pos,
                                       const LQuaternion &quat,
                                       LVecBase3 scale);
  static Transform make_pos_quat_scale_shear(const LVecBase3 &pos,
                                             const LQuaternion &quat,
                                             LVecBase3 scale,
                                             const LVecBase3 &shear);

  INLINE bool is_identity() const;
  INLINE bool is_invalid() const;

  INLINE LPoint3 get_pos() const;
  INLINE void set_pos(const LPoint3 &pos);
  LVecBase3 get_hpr() const;
  void set_hpr(const LVecBase3 &hpr);
  INLINE LQuaternion get_quat() const;
  void set_quat(const LQuaternion &quat);
  LVecBase3 get_scale() const;
  void set_scale(LVecBase3 scale);
  LVecBase3 get_shear() const;
  void set_shear(const LVecBase3 &shear);

  INLINE LMatrix4 get_mat() const;
  INLINE LMatrix3 get_mat3() const;
  INLINE void set_mat3(const LMatrix3 &mat);

  MAKE_PROPERTY(pos, get_pos, set_pos);
  MAKE_PROPERTY(hpr, get_hpr, set_hpr);
  MAKE_PROPERTY(quat, get_quat, set_quat);
  MAKE_PROPERTY(scale, get_scale, set_scale);
  MAKE_PROPERTY(mat, get_mat);
  MAKE_PROPERTY(mat3, get_mat3, set_mat3);

  INLINE Transform compose(const Transform &other) const;
  INLINE Transform invert_compose(const Transform &other) const;
  INLINE Transform get_inverse() const;

  INLINE LPoint3 xform_point(const LPoint3 &point) const;
  INLINE LVector3 xform_vec(const LVector3 &point) const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

public:
  typedef LVecBase4 Column;
  Column _columns[3];

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "Transform");
  }

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const Transform &transform) {
  transform.output(out);
  return out;
}

#include "transform.I"

#endif // !TRANSFORM_H
