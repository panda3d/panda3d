/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file matrixLens.h
 * @author drose
 * @date 2001-12-12
 */

#ifndef MATRIXLENS_H
#define MATRIXLENS_H

#include "pandabase.h"

#include "lens.h"


/**
 * A completely generic linear lens.  This is provided for the benefit of low-
 * level code that wants to specify a perspective or orthographic frustum via
 * an explicit projection matrix, but not mess around with fov's or focal
 * lengths or any of that nonsense.
 */
class EXPCL_PANDA_GOBJ MatrixLens : public Lens {
PUBLISHED:
  INLINE MatrixLens();

public:
  INLINE MatrixLens(const MatrixLens &copy);
  INLINE void operator = (const MatrixLens &copy);

PUBLISHED:
  INLINE void set_user_mat(const LMatrix4 &user_mat);
  INLINE const LMatrix4 &get_user_mat() const;
  MAKE_PROPERTY(user_mat, get_user_mat, set_user_mat);

  INLINE void set_left_eye_mat(const LMatrix4 &user_mat);
  INLINE void clear_left_eye_mat();
  INLINE bool has_left_eye_mat() const;
  INLINE const LMatrix4 &get_left_eye_mat() const;

  INLINE void set_right_eye_mat(const LMatrix4 &user_mat);
  INLINE void clear_right_eye_mat();
  INLINE bool has_right_eye_mat() const;
  INLINE const LMatrix4 &get_right_eye_mat() const;

public:
  virtual PT(Lens) make_copy() const;
  virtual bool is_linear() const;

  virtual void write(std::ostream &out, int indent_level = 0) const;

protected:
  virtual void do_compute_projection_mat(Lens::CData *lens_cdata);

private:
  LMatrix4 _user_mat;
  LMatrix4 _left_eye_mat;
  LMatrix4 _right_eye_mat;

  enum MLFlags {
    MF_has_left_eye    = 0x001,
    MF_has_right_eye   = 0x002,
  };
  int _ml_flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    Lens::init_type();
    register_type(_type_handle, "MatrixLens",
                  Lens::get_class_type());
  }

private:
  static TypeHandle _type_handle;
};

#include "matrixLens.I"

#endif
