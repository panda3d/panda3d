/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file egg_parametrics.cxx
 * @author drose
 * @date 2003-10-13
 */

#include "egg_parametrics.h"
#include "config_egg2pg.h"

/**
 * Returns a new NurbsSurfaceEvaluator that's filled in with the values from
 * the given EggSurface (and transformed by the indicated matrix), or NULL if
 * the object is invalid.  If there is vertex color, it will be applied to
 * values 0 - 3 of the extended vertex values.
 */
PT(NurbsSurfaceEvaluator)
make_nurbs_surface(EggNurbsSurface *egg_surface, const LMatrix4d &mat) {
  if (egg_surface->get_u_order() < 1 || egg_surface->get_u_order() > 4) {
    egg2pg_cat.error()
      << "Invalid NURBSSurface U order for " << egg_surface->get_name() << ": "
      << egg_surface->get_u_order() << "\n";
    return nullptr;
  }

  if (egg_surface->get_v_order() < 1 || egg_surface->get_v_order() > 4) {
    egg2pg_cat.error()
      << "Invalid NURBSSurface V order for " << egg_surface->get_name() << ": "
      << egg_surface->get_v_order() << "\n";
    return nullptr;
  }

  PT(NurbsSurfaceEvaluator) nurbs = new NurbsSurfaceEvaluator;

  nurbs->set_u_order(egg_surface->get_u_order());
  nurbs->set_v_order(egg_surface->get_v_order());

  int num_u_vertices = egg_surface->get_num_u_cvs();
  int num_v_vertices = egg_surface->get_num_v_cvs();
  nurbs->reset(num_u_vertices, num_v_vertices);
  for (int ui = 0; ui < num_u_vertices; ui++) {
    for (int vi = 0; vi < num_v_vertices; vi++) {
      int i = egg_surface->get_vertex_index(ui, vi);
      EggVertex *egg_vertex = egg_surface->get_vertex(i);
      nurbs->set_vertex(ui, vi, LCAST(PN_stdfloat, egg_vertex->get_pos4() * mat));

      LColor color = egg_vertex->get_color();
      nurbs->set_extended_vertices(ui, vi, 0, color.get_data(), 4);
    }
  }

  int num_u_knots = egg_surface->get_num_u_knots();
  if (num_u_knots != nurbs->get_num_u_knots()) {
    egg2pg_cat.error()
      << "Invalid NURBSSurface number of U knots for "
      << egg_surface->get_name() << ": got " << num_u_knots
      << " knots, expected " << nurbs->get_num_u_knots() << "\n";
    return nullptr;
  }

  int num_v_knots = egg_surface->get_num_v_knots();
  if (num_v_knots != nurbs->get_num_v_knots()) {
    egg2pg_cat.error()
      << "Invalid NURBSSurface number of U knots for "
      << egg_surface->get_name() << ": got " << num_v_knots
      << " knots, expected " << nurbs->get_num_v_knots() << "\n";
    return nullptr;
  }

  int i;
  for (i = 0; i < num_u_knots; i++) {
    nurbs->set_u_knot(i, egg_surface->get_u_knot(i));
  }
  for (i = 0; i < num_v_knots; i++) {
    nurbs->set_v_knot(i, egg_surface->get_v_knot(i));
  }

  return nurbs;
}

/**
 * Returns a new NurbsCurveEvaluator that's filled in with the values from the
 * given EggCurve (and transformed by the indicated matrix), or NULL if the
 * object is invalid.  If there is vertex color, it will be applied to values
 * 0 - 3 of the extended vertex values.
 */
PT(NurbsCurveEvaluator)
make_nurbs_curve(EggNurbsCurve *egg_curve, const LMatrix4d &mat) {
  if (egg_curve->get_order() < 1 || egg_curve->get_order() > 4) {
    egg2pg_cat.error()
      << "Invalid NURBSCurve order for " << egg_curve->get_name() << ": "
      << egg_curve->get_order() << "\n";
    return nullptr;
  }

  PT(NurbsCurveEvaluator) nurbs = new NurbsCurveEvaluator;
  nurbs->set_order(egg_curve->get_order());

  nurbs->reset(egg_curve->size());
  EggPrimitive::const_iterator pi;
  int vi = 0;
  for (pi = egg_curve->begin(); pi != egg_curve->end(); ++pi) {
    EggVertex *egg_vertex = (*pi);
    nurbs->set_vertex(vi, LCAST(PN_stdfloat, egg_vertex->get_pos4() * mat));
    LColor color = egg_vertex->get_color();
    nurbs->set_extended_vertices(vi, 0, color.get_data(), 4);
    vi++;
  }

  int num_knots = egg_curve->get_num_knots();
  if (num_knots != nurbs->get_num_knots()) {
    egg2pg_cat.error()
      << "Invalid NURBSCurve number of knots for "
      << egg_curve->get_name() << ": got " << num_knots
      << " knots, expected " << nurbs->get_num_knots() << "\n";
    return nullptr;
  }

  for (int i = 0; i < num_knots; i++) {
    nurbs->set_knot(i, egg_curve->get_knot(i));
  }

  return nurbs;
}
