/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file qtessSurface.cxx
 * @author drose
 * @date 2003-10-13
 */

#include "qtessSurface.h"
#include "qtessGlobals.h"
#include "qtessInputEntry.h"
#include "config_egg_qtess.h"
#include "eggPolygon.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggComment.h"
#include "egg_parametrics.h"
#include "pset.h"
#include "pmap.h"

using std::max;
using std::string;

/**
 *
 */
QtessSurface::
QtessSurface(EggNurbsSurface *egg_surface) :
  _egg_surface(egg_surface)
{
  _nurbs = make_nurbs_surface(_egg_surface, LMatrix4d::ident_mat());
  _has_vertex_color = _egg_surface->has_vertex_color();

  // The first four slots are reserved for vertex color.
  _next_d = 4;

  _importance = 1.0;
  _importance2 = 1.0;
  _match_u = _match_v = nullptr;
  _tess_u = _tess_v = 0;
  _got_scores = false;

  // If the surface is closed in either dimension, the mininum tesselation in
  // that dimension is by default 3, so we don't ribbonize the surface.
  // Otherwise the minimum is 1.
  _min_u = _min_v = 1;
  if (egg_surface->is_closed_u()) {
    _min_u = 3;
  }
  if (egg_surface->is_closed_v()) {
    _min_v = 3;
  }

  if (_nurbs == nullptr) {
    _num_u = _num_v = 0;

  } else {
    record_vertex_extras();

    _nurbs->normalize_u_knots();
    _nurbs->normalize_v_knots();
    _nurbs_result = _nurbs->evaluate();

    _num_u = _nurbs->get_num_u_segments();
    _num_v = _nurbs->get_num_v_segments();

    if (QtessGlobals::_respect_egg) {
      if (egg_surface->get_u_subdiv() != 0) {
        _num_u = egg_surface->get_u_subdiv();
      }
      if (egg_surface->get_v_subdiv() != 0) {
        _num_v = egg_surface->get_v_subdiv();
      }
    }
  }
}

/**
 * Computes the curvature/stretch score for the surface, if it has not been
 * already computed, and returns the net surface score.  This is used both for
 * automatically distributing isoparams among the surfaces by curvature, as
 * well as for automatically placing the isoparams within each surface by
 * curvature.
 */
double QtessSurface::
get_score(double ratio) {
  if (_nurbs == nullptr) {
    return 0.0;
  }

  if (!_got_scores) {
    _u_placer.get_scores(_nurbs->get_num_u_segments() * 100,
                         _nurbs->get_num_v_segments() * 2,
                         ratio, _nurbs_result, true);
    _v_placer.get_scores(_nurbs->get_num_v_segments() * 100,
                         _nurbs->get_num_u_segments() * 2,
                         ratio, _nurbs_result, false);
    _got_scores = true;
  }

  return _u_placer.get_total_score() * _v_placer.get_total_score() * _importance2;
}

/**
 * Applies the appropriate tesselation to the surface, and replaces its node
 * in the tree with an EggGroup containing both the new vertex pool and all of
 * the polygons.
 */
int QtessSurface::
tesselate() {
  apply_match();
  int tris = 0;

  PT(EggGroup) group = do_uniform_tesselate(tris);
  PT(EggNode) new_node = group.p();
  if (new_node == nullptr) {
    new_node = new EggComment(_egg_surface->get_name(),
                              "Omitted NURBS surface.");
    tris = 0;
  }
  EggGroupNode *parent = _egg_surface->get_parent();
  nassertr(parent != nullptr, 0);
  parent->remove_child(_egg_surface);
  parent->add_child(new_node);

  return tris;
}

/**
 * Writes a line to the given output file telling qtess how this surface
 * should be tesselated uniformly.  Returns the number of tris.
 */
int QtessSurface::
write_qtess_parameter(std::ostream &out) {
  apply_match();

  if (_tess_u == 0 || _tess_v == 0) {
    out << get_name() << " : omit\n";

  } else if (_iso_u.empty() || _iso_v.empty()) {
    out << get_name() << " : " << _tess_u << " " << _tess_v << "\n";

  } else {
    out << get_name() << " : " << _iso_u.back() << " " << _iso_v.back();
    QtessInputEntry::output_extra(out, _iso_u, 'u');
    QtessInputEntry::output_extra(out, _iso_v, 'v');
    out << "\n";
  }

  return count_tris();
}


/**
 * Sets up the surface to omit itself from the output.
 */
void QtessSurface::
omit() {
  _tess_u = 0;
  _tess_v = 0;
}

/**
 * Sets the surface up to tesselate itself uniformly at u x v, or if autoplace
 * is true, automatically with u x v quads.
 */
void QtessSurface::
tesselate_uv(int u, int v, bool autoplace, double ratio) {
  _tess_u = u;
  _tess_v = v;
  _iso_u.clear();
  _iso_v.clear();
  if (autoplace) {
    tesselate_auto(_tess_u, _tess_v, ratio);
  }
}

/**
 * Sets the surface up to tesselate itself at specific isoparams only.
 */
void QtessSurface::
tesselate_specific(const pvector<double> &u_list,
                   const pvector<double> &v_list) {
  _iso_u = u_list;
  _iso_v = v_list;
  _tess_u = (int)_iso_u.size() - 1;
  _tess_v = (int)_iso_v.size() - 1;
}

/**
 * Sets the surface up to tesselate itself to a uniform amount per isoparam.
 */
void QtessSurface::
tesselate_per_isoparam(double pi, bool autoplace, double ratio) {
  if (_num_u == 0 || _num_v == 0) {
    omit();

  } else {
    _tess_u = max(_min_u, (int)floor(_num_u * _importance * pi + 0.5));
    _tess_v = max(_min_v, (int)floor(_num_v * _importance * pi + 0.5));
    _iso_u.clear();
    _iso_v.clear();
    if (autoplace) {
      tesselate_auto(_tess_u, _tess_v, ratio);
    }
  }
}


/**
 * Sets the surface up to tesselate itself according to its computed curvature
 * score in both dimensions.
 */
void QtessSurface::
tesselate_per_score(double pi, bool autoplace, double ratio) {
  if (get_score(ratio) <= 0.0) {
    omit();

  } else {
    _tess_u = max(_min_u, (int)floor(_u_placer.get_total_score() * _importance * pi + 0.5));
    _tess_v = max(_min_v, (int)floor(_v_placer.get_total_score() * _importance * pi + 0.5));
    _iso_u.clear();
    _iso_v.clear();
    if (autoplace) {
      tesselate_auto(_tess_u, _tess_v, ratio);
    }
  }
}

/**
 * Sets the surface up to tesselate itself by automatically determining the
 * best place to put the indicated u x v isoparams.
 */
void QtessSurface::
tesselate_auto(int u, int v, double ratio) {
  if (get_score(ratio) <= 0.0) {
    omit();

  } else {
    _u_placer.place(u, _iso_u);
    _v_placer.place(v, _iso_v);
    _tess_u = (int)_iso_u.size() - 1;
    _tess_v = (int)_iso_v.size() - 1;
  }
}

/**
 * Records the joint membership and morph offsets of each control vertex in
 * the extra-dimensional space of the NURBS, so that we can extract this data
 * out again later to apply to the polygon vertices.
 */
void QtessSurface::
record_vertex_extras() {
  int num_u_vertices = _egg_surface->get_num_u_cvs();
  int num_v_vertices = _egg_surface->get_num_v_cvs();

  for (int ui = 0; ui < num_u_vertices; ui++) {
    for (int vi = 0; vi < num_v_vertices; vi++) {
      int i = _egg_surface->get_vertex_index(ui, vi);
      EggVertex *egg_vertex = _egg_surface->get_vertex(i);

      // The joint membership.
      EggVertex::GroupRef::const_iterator gi;
      for (gi = egg_vertex->gref_begin(); gi != egg_vertex->gref_end(); ++gi) {
        EggGroup *joint = (*gi);
        int d = get_joint_membership_index(joint);
        double membership = joint->get_vertex_membership(egg_vertex);
        _nurbs->set_extended_vertex(ui, vi, d, membership);
      }

      // The xyz morphs.
      EggMorphVertexList::const_iterator dxi;
      for (dxi = egg_vertex->_dxyzs.begin();
           dxi != egg_vertex->_dxyzs.end();
           ++dxi) {
        const string &morph_name = (*dxi).get_name();
        LVector3 delta = LCAST(PN_stdfloat, (*dxi).get_offset());
        int d = get_dxyz_index(morph_name);
        _nurbs->set_extended_vertices(ui, vi, d, delta.get_data(), 3);
      }

      // The rgba morphs.
      EggMorphColorList::const_iterator dri;
      for (dri = egg_vertex->_drgbas.begin();
           dri != egg_vertex->_drgbas.end();
           ++dri) {
        const string &morph_name = (*dri).get_name();
        const LVector4 &delta = (*dri).get_offset();
        int d = get_drgba_index(morph_name);
        _nurbs->set_extended_vertices(ui, vi, d, delta.get_data(), 4);
      }
    }
  }
}

/**
 * If the surface was set up to copy its tesselation in either axis from
 * another surface, makes this copy now.
 */
void QtessSurface::
apply_match() {
  if (_match_u != nullptr) {
    QtessSurface *m = *_match_u;
    if (m == nullptr) {
      qtess_cat.warning()
        << "No surface to match " << get_name() << " to in U.\n";
    } else {
      if (qtess_cat.is_debug()) {
        qtess_cat.debug()
          << "Matching " << get_name() << " in U to " << m->get_name()
          << " in " << (_match_u_to_u?'U':'V') << ".\n";
      }
      if (_match_u_to_u) {
        _tess_u = m->_tess_u;
        _iso_u = m->_iso_u;
      } else {
        _tess_u = m->_tess_v;
        _iso_u = m->_iso_v;
      }
    }
  }

  if (_match_v != nullptr) {
    QtessSurface *m = *_match_v;
    if (m == nullptr) {
      qtess_cat.warning()
        << "No surface to match " << get_name() << " in V.\n";
    } else {
      if (qtess_cat.is_debug()) {
        qtess_cat.debug()
          << "Matching " << get_name() << " in V to " << m->get_name()
          << " in " << (_match_v_to_v?'V':'U') << ".\n";
      }
      if (_match_v_to_v) {
        _tess_v = m->_tess_v;
        _iso_v = m->_iso_v;
      } else {
        _tess_v = m->_tess_u;
        _iso_v = m->_iso_u;
      }
    }
  }
}

/**
 * Subdivide the surface uniformly according to the parameters specified by an
 * earlier call to omit(), teseselate_uv(), or tesselate_per_isoparam().
 */
PT(EggGroup) QtessSurface::
do_uniform_tesselate(int &tris) const {
  tris = 0;

  if (_tess_u == 0 || _tess_v == 0) {
    // No tesselation!
    if (qtess_cat.is_debug()) {
      qtess_cat.debug()
        << get_name() << " : omit\n";
    }
    return nullptr;
  }

  PT(EggGroup) group = new EggGroup(_egg_surface->get_name());

  // _tess_u and _tess_v are the number of patches to create.  Convert that to
  // the number of vertices.

  int num_u = _tess_u + 1;
  int num_v = _tess_v + 1;

  if (qtess_cat.is_debug()) {
    qtess_cat.debug() << get_name() << " : " << tris << "\n";
  }

  assert(_iso_u.empty() || (int)_iso_u.size() == num_u);
  assert(_iso_v.empty() || (int)_iso_v.size() == num_v);

  // Now how many vertices is that total, and how many vertices per strip?
  int num_verts = num_u * num_v;

  // Create a vertex pool.
  PT(EggVertexPool) vpool = new EggVertexPool(_egg_surface->get_name());
  group->add_child(vpool);

  // Create all the vertices.
  int ui, vi;
  double u, v;

  typedef pvector<EggVertex *> VertexList;
  VertexList new_verts;
  new_verts.reserve(num_verts);

  // Also collect the vertices into this set to group them by spatial position
  // only.  This is relevant for calculating normals.
  typedef pset<EggVertex *> NVertexGroup;
  typedef pmap<LVertexd, NVertexGroup> NVertexCollection;
  NVertexCollection n_collection;

  for (vi = 0; vi < num_v; vi++) {
    if (_iso_v.empty()) {
      v = (double)vi / (double)(num_v-1);
    } else {
      v = _iso_v[vi] / _iso_v.back();
    }
    for (ui = 0; ui < num_u; ui++) {
      if (_iso_u.empty()) {
        u = (double)ui / (double)(num_u-1);
      } else {
        u = _iso_u[ui] / _iso_u.back();
      }

      PT(EggVertex) egg_vertex = evaluate_vertex(u, v);
      vpool->add_vertex(egg_vertex);
      new_verts.push_back(egg_vertex);
      n_collection[egg_vertex->get_pos3()].insert(egg_vertex);
    }
  }
  nassertr((int)new_verts.size() == num_verts, nullptr);

  // Now create a bunch of quads.
  for (vi = 1; vi < num_v; vi++) {
    for (ui = 1; ui < num_u; ui++) {
      PT(EggPolygon) poly = new EggPolygon;
      poly->add_vertex(new_verts[vi*num_u + (ui-1)]);
      poly->add_vertex(new_verts[(vi-1)*num_u + (ui-1)]);
      poly->add_vertex(new_verts[(vi-1)*num_u + ui]);
      poly->add_vertex(new_verts[vi*num_u + ui]);

      poly->copy_attributes(*_egg_surface);

      // We compute a polygon normal just so we can verify the calculated
      // vertex normals.  It's also helpful for identifying degenerate
      // polygons.
      if (poly->recompute_polygon_normal()) {
        tris += 2;
        group->add_child(poly);
      }
    }
  }

  // Now check all the vertex normals by comparing them to the polygon
  // normals.  Some might have not been computed at all; others might be
  // facing in the wrong direction.

  // Now go back through and normalize the computed normals.
  NVertexCollection::const_iterator nci;
  for (nci = n_collection.begin(); nci != n_collection.end(); ++nci) {
    const NVertexGroup &group = (*nci).second;

    // Calculate the normal these vertices should have based on the polygons
    // that share it.
    LNormald normal = LNormald::zero();
    int num_polys = 0;
    NVertexGroup::const_iterator ngi;
    for (ngi = group.begin(); ngi != group.end(); ++ngi) {
      EggVertex *egg_vertex = (*ngi);
      EggVertex::PrimitiveRef::const_iterator pri;
      for (pri = egg_vertex->pref_begin();
           pri != egg_vertex->pref_end();
           ++pri) {
        EggPrimitive *egg_primitive = (*pri);
        nassertr(egg_primitive->has_normal(), nullptr);
        normal += egg_primitive->get_normal();
        num_polys++;
      }
    }

    if (num_polys > 0) {
      normal /= (double)num_polys;

      // Now compare this normal with what the NURBS representation
      // calculated.  It should be facing in at least vaguely the same
      // direction.
      for (ngi = group.begin(); ngi != group.end(); ++ngi) {
        EggVertex *egg_vertex = (*ngi);
        if (egg_vertex->has_normal()) {
          if (normal.dot(egg_vertex->get_normal()) < 0.0) {
            // This one is backwards.
            egg_vertex->set_normal(-egg_vertex->get_normal());
          }
        } else {
          // This vertex doesn't have a normal; it gets the computed normal.
          egg_vertex->set_normal(normal);
        }
      }
    }
  }

  return group;
}

/**
 * Evaluates the surface at the given u, v position and sets the vertex to the
 * appropriate values.  Also sets the joint membership of the vertex.
 */
PT(EggVertex) QtessSurface::
evaluate_vertex(double u, double v) const {
  PT(EggVertex) egg_vertex = new EggVertex;

  LVertex point;
  LNormal normal;
  _nurbs_result->eval_point(u, v, point);
  _nurbs_result->eval_normal(u, v, normal);

  // If the normal is too short, don't consider it--it's probably inaccurate
  // due to numerical limitations.  We'll recompute it later based on the
  // polygon normals.
  PN_stdfloat length = normal.length();
  if (length > 0.0001f) {
    normal /= length;
    egg_vertex->set_normal(LCAST(double, normal));
  }

  egg_vertex->set_pos(LCAST(double, point));
  egg_vertex->set_uv(LVecBase2d(u, v));

  // The color is stored, by convention, in slots 0-4 of the surface.
  if (_has_vertex_color) {
    LColor rgba;
    _nurbs_result->eval_extended_points(u, v, 0, &rgba[0], 4);
    egg_vertex->set_color(rgba);
  }

  // Also fill in the joint membership.
  JointTable::const_iterator jti;
  for (jti = _joint_table.begin(); jti != _joint_table.end(); ++jti) {
    EggGroup *joint = (*jti).first;
    int d = (*jti).second;

    double membership = _nurbs_result->eval_extended_point(u, v, d);
    if (membership > 0.0) {
      joint->ref_vertex(egg_vertex, membership);
    }
  }

  // And the morphs.
  MorphTable::const_iterator mti;
  for (mti = _dxyz_table.begin(); mti != _dxyz_table.end(); ++mti) {
    const string &morph_name = (*mti).first;
    int d = (*mti).second;

    LVector3 delta;
    _nurbs_result->eval_extended_points(u, v, d, &delta[0], 3);
    if (!delta.almost_equal(LVector3::zero())) {
      egg_vertex->_dxyzs.insert(EggMorphVertex(morph_name, LCAST(double, delta)));
    }
  }

  for (mti = _drgba_table.begin(); mti != _drgba_table.end(); ++mti) {
    const string &morph_name = (*mti).first;
    int d = (*mti).second;

    LVector4 delta;
    _nurbs_result->eval_extended_points(u, v, d, &delta[0], 4);
    if (!delta.almost_equal(LVector4::zero())) {
      egg_vertex->_drgbas.insert(EggMorphColor(morph_name, delta));
    }
  }

  return egg_vertex;
}
