/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggCompositePrimitive.h
 * @author drose
 * @date 2005-03-13
 */

#ifndef EGGCOMPOSITEPRIMITIVE_H
#define EGGCOMPOSITEPRIMITIVE_H

#include "pandabase.h"

#include "eggPrimitive.h"

/**
 * The base class for primitives such as triangle strips and triangle fans,
 * which include several component triangles, each of which might have its own
 * color and/or normal.
 */
class EXPCL_PANDA_EGG EggCompositePrimitive : public EggPrimitive {
PUBLISHED:
  INLINE explicit EggCompositePrimitive(const std::string &name = "");
  INLINE EggCompositePrimitive(const EggCompositePrimitive &copy);
  INLINE EggCompositePrimitive &operator = (const EggCompositePrimitive &copy);
  virtual ~EggCompositePrimitive();

  virtual Shading get_shading() const;

  INLINE size_t get_num_components() const;
  INLINE const EggAttributes *get_component(size_t i) const;
  INLINE EggAttributes *get_component(size_t i);
  MAKE_SEQ(get_components, get_num_components, get_component);
  INLINE void set_component(size_t i, const EggAttributes *attrib);

  MAKE_SEQ_PROPERTY(components, get_num_components, get_component, set_component);

  INLINE bool triangulate_into(EggGroupNode *container) const;
  PT(EggCompositePrimitive) triangulate_in_place();

  virtual void unify_attributes(Shading shading);
  virtual void apply_last_attribute();
  virtual void apply_first_attribute();
  virtual void post_apply_flat_attribute();
  virtual bool cleanup();

protected:
  virtual int get_num_lead_vertices() const=0;
  virtual void prepare_add_vertex(EggVertex *vertex, int i, int n);
  virtual void prepare_remove_vertex(EggVertex *vertex, int i, int n);

  virtual bool do_triangulate(EggGroupNode *container) const;

  void write_body(std::ostream &out, int indent_level) const;

private:
  typedef pvector<EggAttributes *> Components;
  Components _components;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggPrimitive::init_type();
    register_type(_type_handle, "EggCompositePrimitive",
                  EggPrimitive::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "eggCompositePrimitive.I"

#endif
