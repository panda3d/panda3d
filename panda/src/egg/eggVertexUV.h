/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexUV.h
 * @author drose
 * @date 2004-07-20
 */

#ifndef EGGVERTEXUV_H
#define EGGVERTEXUV_H

#include "pandabase.h"

#include "eggMorphList.h"
#include "eggNamedObject.h"

#include "luse.h"

/**
 * The set of UV's that may or may not be assigned to a vertex.  To support
 * multitexturing, there may be multiple sets of UV's on a particular vertex,
 * each with its own name.
 */
class EXPCL_PANDA_EGG EggVertexUV : public EggNamedObject {
PUBLISHED:
  explicit EggVertexUV(const std::string &name, const LTexCoordd &uv);
  explicit EggVertexUV(const std::string &name, const LTexCoord3d &uvw);
  EggVertexUV(const EggVertexUV &copy);
  EggVertexUV &operator = (const EggVertexUV &copy);
  virtual ~EggVertexUV();

  INLINE static std::string filter_name(const std::string &name);
  INLINE void set_name(const std::string &name);

  INLINE int get_num_dimensions() const;
  INLINE bool has_w() const;
  INLINE LTexCoordd get_uv() const;
  INLINE const LTexCoord3d &get_uvw() const;
  INLINE void set_uv(const LTexCoordd &texCoord);
  INLINE void set_uvw(const LTexCoord3d &texCoord);

  INLINE bool has_tangent() const;
  INLINE bool has_tangent4() const;
  INLINE const LNormald &get_tangent() const;
  INLINE LVecBase4d get_tangent4() const;
  INLINE void set_tangent(const LNormald &tangent);
  INLINE void set_tangent4(const LVecBase4d &tangent);
  INLINE void clear_tangent();

  INLINE bool has_binormal() const;
  INLINE const LNormald &get_binormal() const;
  INLINE void set_binormal(const LNormald &binormal);
  INLINE void clear_binormal();

  static PT(EggVertexUV) make_average(const EggVertexUV *first,
                                      const EggVertexUV *second);

  void transform(const LMatrix4d &mat);

  void write(std::ostream &out, int indent_level) const;
  int compare_to(const EggVertexUV &other) const;

  EggMorphTexCoordList _duvs;

private:
  enum Flags {
    F_has_tangent   = 0x001,
    F_has_binormal  = 0x002,
    F_has_w         = 0x004,
    F_has_tangent4  = 0x008,

    // Only defined temporarily as we can't add a float to this class in 1.10.
    F_flip_computed_binormal = 0x010,
  };

  int _flags;
  LNormald _tangent;
  LNormald _binormal;
  LTexCoord3d _uvw;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNamedObject::init_type();
    register_type(_type_handle, "EggVertexUV",
                  EggNamedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggVertexUV.I"

#endif
