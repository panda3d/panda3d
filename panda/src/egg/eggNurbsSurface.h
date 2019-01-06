/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggNurbsSurface.h
 * @author drose
 * @date 2000-02-15
 */

#ifndef EGGNURBSSURFACE_H
#define EGGNURBSSURFACE_H

#include "pandabase.h"

#include "eggSurface.h"
#include "eggNurbsCurve.h"
#include "vector_double.h"
#include "plist.h"

/**
 * A parametric NURBS surface.
 */
class EXPCL_PANDA_EGG EggNurbsSurface : public EggSurface {
PUBLISHED:
  typedef plist< PT(EggNurbsCurve) > Curves;
  typedef Curves Loop;
  typedef plist<Loop> Loops;
  typedef Loops Trim;
  typedef plist<Trim> Trims;

  INLINE explicit EggNurbsSurface(const std::string &name = "");
  INLINE EggNurbsSurface(const EggNurbsSurface &copy);
  INLINE EggNurbsSurface &operator = (const EggNurbsSurface &copy);

  virtual EggNurbsSurface *make_copy() const override;

  void setup(int u_order, int v_order,
             int num_u_knots, int num_v_knots);

  INLINE void set_u_order(int u_order);
  INLINE void set_v_order(int v_order);
  void set_num_u_knots(int num);
  void set_num_v_knots(int num);

  INLINE void set_u_knot(int k, double value);
  INLINE void set_v_knot(int k, double value);
  INLINE void set_cv(int ui, int vi, EggVertex *vertex);

  bool is_valid() const;

  INLINE int get_u_order() const;
  INLINE int get_v_order() const;
  INLINE int get_u_degree() const;
  INLINE int get_v_degree() const;
  INLINE int get_num_u_knots() const;
  INLINE int get_num_v_knots() const;
  INLINE int get_num_u_cvs() const;
  INLINE int get_num_v_cvs() const;
  INLINE int get_num_cvs() const;

  INLINE int get_u_index(int vertex_index) const;
  INLINE int get_v_index(int vertex_index) const;
  INLINE int get_vertex_index(int ui, int vi) const;

  bool is_closed_u() const;
  bool is_closed_v() const;

  INLINE double get_u_knot(int k) const;
  MAKE_SEQ(get_u_knots, get_num_u_knots, get_u_knot);
  INLINE double get_v_knot(int k) const;
  MAKE_SEQ(get_v_knots, get_num_v_knots, get_v_knot);
  INLINE EggVertex *get_cv(int ui, int vi) const;

  virtual void write(std::ostream &out, int indent_level) const override;

public:
  Curves _curves_on_surface;
  Trims _trims;

protected:
  virtual void r_apply_texmats(EggTextureCollection &textures) override;

private:
  typedef vector_double Knots;
  Knots _u_knots;
  Knots _v_knots;
  int _u_order;
  int _v_order;

public:

  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    EggSurface::init_type();
    register_type(_type_handle, "EggNurbsSurface",
                  EggSurface::get_class_type());
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

#include "eggNurbsSurface.I"

#endif
