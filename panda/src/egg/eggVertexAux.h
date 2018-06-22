/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggVertexAux.h
 * @author jenes
 * @date 2011-11-15
 */

#ifndef EGGVERTEXAUX_H
#define EGGVERTEXAUX_H

#include "pandabase.h"

#include "eggMorphList.h"
#include "eggNamedObject.h"

#include "luse.h"

/**
 * The set of named auxiliary data that may or may not be assigned to a
 * vertex.  Panda will import this data and create a custom column for it in
 * the vertex data, but will not otherwise interpret it.  Presumably, a shader
 * will process the data later.
 */
class EXPCL_PANDA_EGG EggVertexAux : public EggNamedObject {
PUBLISHED:
  explicit EggVertexAux(const std::string &name, const LVecBase4d &aux);
  EggVertexAux(const EggVertexAux &copy);
  EggVertexAux &operator = (const EggVertexAux &copy);
  virtual ~EggVertexAux();

  INLINE void set_name(const std::string &name);

  INLINE const LVecBase4d &get_aux() const;
  INLINE void set_aux(const LVecBase4d &aux);

  static PT(EggVertexAux) make_average(const EggVertexAux *first,
                                       const EggVertexAux *second);

  void write(std::ostream &out, int indent_level) const;
  int compare_to(const EggVertexAux &other) const;

private:
  LVecBase4d _aux;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggNamedObject::init_type();
    register_type(_type_handle, "EggVertexAux",
                  EggNamedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "eggVertexAux.I"

#endif
