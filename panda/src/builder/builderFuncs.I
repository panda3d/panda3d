// Filename: builderFuncs.I
// Created by:  drose (09Sep97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "builderPrim.h"
#include "mesherTempl.h"
#include "builderNormalVisualizer.h"
#include "config_builder.h"
#include "geom.h"
#include "geomprimitives.h"

#include <algorithm>

struct DecompVtx {
  int index;
  BuilderV coord;
  struct DecompVtx *next;
};

////////////////////////////////////////////////////////////////////
//     Function: decomp_concave
//  Description: Decomposes a concave polygon into triangles.  Returns
//               true if successful, false if the polygon is
//               self-intersecting.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
static bool
decomp_concave(const PrimType &prim, BuilderBucket &bucket,
               OutputIterator result,
               int asum, int x, int y) {
#define VX(p, c)    p->coord[c]

  pvector<PrimType> output_prims;

  DecompVtx *p0, *p1, *p2, *t0, *vert;
  DecompVtx *m[3];
  float xmin, xmax, ymin, ymax;
  int i, init, csum, chek;
  float a[3], b[3], c[3], s[3];

  int num_verts = prim.get_num_verts();
  nassertr(num_verts >= 3, false);

  /* Make linked list of verts */
  vert = (DecompVtx *) alloca(sizeof(DecompVtx));
  vert->index = 0;
  vert->coord = prim.get_vertex(0).get_coord_value(bucket);
  p1 = vert;

  for (i = 1; i < num_verts; i++) {
    p0 = (DecompVtx *) alloca(sizeof(DecompVtx));
    p0->index = i;
    p0->coord = prim.get_vertex(i).get_coord_value(bucket);
    // There shouldn't be two consecutive identical vertices.  If
    // there are, skip one.
    if (!(p0->coord == p1->coord)) {
      p1->next = p0;
      p1 = p0;
    }
  }
  p1->next = vert;

  p0 = vert;
  p1 = p0->next;
  p2 = p1->next;
  m[0] = p0;
  m[1] = p1;
  m[2] = p2;
  chek = 0;

  while (p0 != p2->next) {
    /* Polygon is self-intersecting so punt */
    if (chek &&
        m[0] == p0 &&
        m[1] == p1 &&
        m[2] == p2) {

      //      builder_cat.info() << "Could not decompose concave polygon!";
      return false;
    }

    chek = 1;

    a[0] = VX(p1, y) - VX(p2, y);
    b[0] = VX(p2, x) - VX(p1, x);
    a[2] = VX(p0, y) - VX(p1, y);
    b[2] = VX(p1, x) - VX(p0, x);

    csum = ((b[0] * a[2] - b[2] * a[0] >= 0.0) ? 1 : 0);

    if (csum ^ asum) {
      /* current angle is concave */
      p0 = p1;
      p1 = p2;
      p2 = p2->next;

    } else {
      /* current angle is convex */
      xmin = (VX(p0, x) < VX(p1, x)) ? VX(p0, x) : VX(p1, x);
      if (xmin > VX(p2, x))
        xmin = VX(p2, x);

      xmax = (VX(p0, x) > VX(p1, x)) ? VX(p0, x) : VX(p1, x);
      if (xmax < VX(p2, x))
        xmax = VX(p2, x);

      ymin = (VX(p0, y) < VX(p1, y)) ? VX(p0, y) : VX(p1, y);
      if (ymin > VX(p2, y))
        ymin = VX(p2, y);

      ymax = (VX(p0, y) > VX(p1, y)) ? VX(p0, y) : VX(p1, y);
      if (ymax < VX(p2, y))
        ymax = VX(p2, y);

      for (init = 1, t0 = p2->next; t0 != p0; t0 = t0->next) {
        if (VX(t0, x) >= xmin && VX(t0, x) <= xmax &&
            VX(t0, y) >= ymin && VX(t0, y) <= ymax) {
          if (init) {
            a[1] = VX(p2, y) - VX(p0, y);
            b[1] = VX(p0, x) - VX(p2, x);
            init = 0;
            c[0] = VX(p1, x) * VX(p2, y) - VX(p2, x) * VX(p1, y);
            c[1] = VX(p2, x) * VX(p0, y) - VX(p0, x) * VX(p2, y);
            c[2] = VX(p0, x) * VX(p1, y) - VX(p1, x) * VX(p0, y);
          }

          s[0] = a[0] * VX(t0, x) + b[0] * VX(t0, y) + c[0];
          s[1] = a[1] * VX(t0, x) + b[1] * VX(t0, y) + c[1];
          s[2] = a[2] * VX(t0, x) + b[2] * VX(t0, y) + c[2];

          if (asum) {
            if (s[0] >= 0.0 && s[1] >= 0.0 && s[2] >= 0.0)
              break;
          } else {
            if (s[0] <= 0.0 && s[1] <= 0.0 && s[2] <= 0.0)
              break;
          }
        }
      }

      if (t0 != p0) {
        p0 = p1;
        p1 = p2;
        p2 = p2->next;
      } else {
        PrimType new_prim(prim);
        new_prim.set_type(BPT_tri);
        new_prim.clear_vertices();
        new_prim.add_vertex(prim.get_vertex(p0->index));
        new_prim.add_vertex(prim.get_vertex(p1->index));
        new_prim.add_vertex(prim.get_vertex(p2->index));
        output_prims.push_back(new_prim);

        p0->next = p1->next;
        p1 = p2;
        p2 = p2->next;

        m[0] = p0;
        m[1] = p1;
        m[2] = p2;
        chek = 0;
      }
    }
  }

  PrimType new_prim(prim);
  new_prim.set_type(BPT_tri);
  new_prim.clear_vertices();
  new_prim.add_vertex(prim.get_vertex(p0->index));
  new_prim.add_vertex(prim.get_vertex(p1->index));
  new_prim.add_vertex(prim.get_vertex(p2->index));
  output_prims.push_back(new_prim);

  copy(output_prims.begin(), output_prims.end(), result);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: triangulate_poly
//  Description: Breaks a (possibly concave) higher-order polygon into
//               a series of constituent triangles.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
static bool
triangulate_poly(const PrimType &prim, BuilderBucket &bucket,
                 OutputIterator result) {
  BuilderV p0, p1, as;
  float dx1, dy1, dx2, dy2, max;
  int i, flag, asum, csum, index, x, y, v0, v1, v, even;

  // First see if the polygon is just a triangle
  int num_verts = prim.get_num_verts();
  if (num_verts == 3) {
    PrimType new_prim(prim);
    new_prim.set_type(BPT_tri);
    *result++ = new_prim;

    return true;

  } else if (num_verts < 3) {
    // Or if it's a degenerate polygon.
    return false;
  }

  // calculate signed areas
  as[0] = 0.0;
  as[1] = 0.0;
  as[2] = 0.0;

  for (i = 0; i < num_verts; i++) {
    p0 = prim.get_vertex(i).get_coord_value(bucket);
    p1 = prim.get_vertex((i + 1) % num_verts).get_coord_value(bucket);
    as[0] += p0[0] * p1[1] - p0[1] * p1[0];
    as[1] += p0[0] * p1[2] - p0[2] * p1[0];
    as[2] += p0[1] * p1[2] - p0[2] * p1[1];
  }

  /* select largest signed area */
  max = 0.0;
  index = 0;
  flag = 0;
  for (i = 0; i < 3; i++) {
    if (as[i] >= 0.0) {
      if (as[i] > max) {
        max = as[i];
        index = i;
        flag = 1;
      }
    } else {
      as[i] = -as[i];
      if (as[i] > max) {
        max = as[i];
        index = i;
        flag = 0;
      }
    }
  }

  /* pointer offsets */
  switch (index) {
  case 0:
    x = 0;
    y = 1;
    break;

  case 1:
    x = 0;
    y = 2;
    break;

  default: // case 2
    x = 1;
    y = 2;
    break;
  }

  /* concave check */
  p0 = prim.get_vertex(0).get_coord_value(bucket);
  p1 = prim.get_vertex(1).get_coord_value(bucket);
  dx1 = p1[x] - p0[x];
  dy1 = p1[y] - p0[y];
  p0 = p1;
  p1 = prim.get_vertex(2).get_coord_value(bucket);

  dx2 = p1[x] - p0[x];
  dy2 = p1[y] - p0[y];
  asum = ((dx1 * dy2 - dx2 * dy1 >= 0.0) ? 1 : 0);

  for (i = 0; i < num_verts - 1; i++) {
    p0 = p1;
    p1 = prim.get_vertex((i+3) % num_verts).get_coord_value(bucket);

    dx1 = dx2;
    dy1 = dy2;
    dx2 = p1[x] - p0[x];
    dy2 = p1[y] - p0[y];
    csum = ((dx1 * dy2 - dx2 * dy1 >= 0.0) ? 1 : 0);

    if (csum ^ asum) {
      return decomp_concave(prim, bucket, result, flag, x, y);
    }
  }

  v0 = 0;
  v1 = 1;
  v = num_verts - 1;

  even = 1;

  /*
   * Convert to triangles only. Do not fan out from a single vertex
   * but zigzag into triangle strip.
   */
  for (i = 0; i < num_verts - 2; i++) {
    if (even) {
      PrimType new_prim(prim);
      new_prim.set_type(BPT_tri);
      new_prim.clear_vertices();
      new_prim.add_vertex(prim.get_vertex(v0));
      new_prim.add_vertex(prim.get_vertex(v1));
      new_prim.add_vertex(prim.get_vertex(v));
      *result++ = new_prim;
      v0 = v1;
      v1 = v;
      v = v0 + 1;
    } else {
      PrimType new_prim(prim);
      new_prim.set_type(BPT_tri);
      new_prim.clear_vertices();
      new_prim.add_vertex(prim.get_vertex(v1));
      new_prim.add_vertex(prim.get_vertex(v0));
      new_prim.add_vertex(prim.get_vertex(v));
      *result++ = new_prim;
      v0 = v1;
      v1 = v;
      v = v0 - 1;
    }

    even = !even;
  }

  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: expand_polys
//  Description: Identifies a single polygon as a triangle, quad, or
//               higher-order polygon, and writes it into the result
//               list.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
static bool
expand_polys(PrimType &prim, BuilderBucket &,
             OutputIterator result) {
  switch (prim.get_num_verts()) {
  case 0:
  case 1:
  case 2:
    return false;

  case 3:
    prim.set_type(BPT_tri);
    break;

  case 4:
    prim.set_type(BPT_quad);
    break;

  default:
    prim.set_type(BPT_poly);
  }

  *result++ = prim;
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: expand_points
//  Description: Expands a light points primitive into its individual
//               component points, with one point per primitive.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
static bool
expand_points(const PrimType &prim, BuilderBucket &,
              OutputIterator result) {
  // Each vertex goes in its own primitive.

  int num_verts = prim.get_num_verts();
  for (int i = 0; i < num_verts; i++) {
    PrimType new_prim(prim);
    new_prim.clear_vertices();
    new_prim.add_vertex(prim.get_vertex(i));
    *result++ = new_prim;
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: expand_lines
//  Description: Expands a linestrip primitive into its component line
//               primitives.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
static bool
expand_lines(PrimType &prim, BuilderBucket &,
             OutputIterator result) {
  // Actually, we don't have support for meshing linestrips right now,
  // so let's not break up the linestrips we're supplied with.
  /*
  if (bucket._subdivide_polys) {
    // If we're subdividing, each line segment goes in its own
    // primitive.  This breaks up the linestrips already defined;
    // we'll re-strip them later if the generate-tstrips flag is
    // enabled.
    prim.set_type(BPT_line);
    
    int num_verts = prim.get_num_verts();
    for (int i = 1; i < num_verts; i++) {
      PrimType new_prim(prim);
      new_prim.clear_vertices();
      new_prim.add_vertex(prim.get_vertex(i-1));
      new_prim.add_vertex(prim.get_vertex(i));
      *result++ = new_prim;
    }
    return true;
  }
  */

  // If we're not to subdivide the polys, then just pass them through
  // as they are.  Two vertices is a BPT_line; more than that is a
  // BPT_linestrip.
  if (prim.get_num_verts() > 2) {
    prim.set_type(BPT_linestrip);
  } else {
    prim.set_type(BPT_line);
  }
  *result++ = prim;
  
  return true;
}



////////////////////////////////////////////////////////////////////
//     Function: expand
//  Description: Receives a single primitive as a BuilderPrim or
//               BuilderPrimI object, as input by the user.  Does some
//               initial processing on the primitive to verify
//               internal consistency (for instance, that a quad has
//               four vertices), and returns a new BuilderPrim or
//               series of BuilderPrim objects, suitable for building
//               with.
//
//               More than one primitive might be returned because
//               higher-order polygons may be broken up into
//               triangles, and linestrips and points are broken into
//               their component pieces.  The output primitives are
//               written into the STL container defined by result.
////////////////////////////////////////////////////////////////////
template <class PrimType, class OutputIterator>
bool
expand(const PrimType &prim, BuilderBucket &bucket, OutputIterator result) {
  // Make a copy of the prim so we can fiddle with it.
  PrimType new_prim = prim;

  switch (new_prim.get_type()) {
  case BPT_poly:
  case BPT_tri:
  case BPT_quad:
    // These three types are all treated the same, as polygons.  We
    // don't entirely trust the user to match the polygon type with
    // the number of verts, so we'll do it ourselves later.
    new_prim.remove_doubled_verts(true);

    if (!bucket._subdivide_polys ||
        (bucket._mesh && new_prim.get_num_verts() <= 4)) {
      // If we're meshing, we'd like to send quads through without
      // subdividing.  The mesher can take advantage of the extra
      // information (and will eventually produce tris anyway).
      return expand_polys(new_prim, bucket, result);
    } else {
      // If we're not meshing, we'll break them into tris now.
      return triangulate_poly(new_prim, bucket, result);
    }

  case BPT_point:
    new_prim.remove_doubled_verts(false);
    return expand_points(new_prim, bucket, result);

  case BPT_line:
  case BPT_linestrip:
    new_prim.remove_doubled_verts(false);
    return expand_lines(new_prim, bucket, result);

  default:
    builder_cat.error() << "Unknown prim type\n";
    return false;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: build_geoms
//  Description: Accepts a list of BuilderPrim or BuilderPrimI
//               objects, defined by the iterators first and last, and
//               creates corresponding geometry for them in the
//               indicated GeomNode.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class PrimType>
static int
build_geoms(InputIterator first, InputIterator last,
            BuilderBucket &bucket, GeomNode *geom_node,
            PrimType *) {
  if (first==last) {
    return 0;
  }

  // By the time we get here, we have a list of primitives that all have
  // the same properties:

  // 1. The BuilderBucket.
  // 2. The indexed/nonindexed type.
  // 3. The primitive type (polygon, line, point).
  // 4. The pixel size.

  // The binding of normals, colors, or texcoords: per-vertex,
  // per-prim, or per-component, will generally be the same across all
  // primitives, but it might not always be the same, because in
  // certain special cases the mesher might have changed these
  // properties when it built tristrips.

  typedef TYPENAME PrimType::VType VType;
  typedef TYPENAME PrimType::NType NType;
  typedef TYPENAME PrimType::TType TType;
  typedef TYPENAME PrimType::CType CType;

  // We need to determine the common binding type for all primitives.
  // For a given attribute, say normals, there are at most five cases,
  // in the order of priority:
  //
  //  1. If at least one primitive in the list does not have normals,
  //     the binding will be G_OFF.
  //
  //  2. If at least one primitive has per-vertex normals, the binding
  //     will be G_PER_VERTEX.
  //
  //  3. If at least one primitive has per-component normals, the
  //     binding will be G_PER_COMPONENT.
  //
  //  4. If none of the first three apply, it follows that all
  //     primitives have an overall normal.  If any primitive's
  //     overall normal differs from any other, the binding will be
  //     G_PER_PRIM.
  //
  //  5. If none of the above apply, the binding will be G_OVERALL.
  //
  // An exception to the above is for texcoords, which is either G_OFF
  // (by rule 1) or G_PER_VERTEX.

  GeomBindType bind_normals = G_OVERALL;
  GeomBindType bind_colors = G_OVERALL;
  GeomBindType bind_texcoords = G_PER_VERTEX;

  NType overall_normal(0);
  CType overall_color(0);
  bool first_normal = true;
  bool first_color = true;

  InputIterator i;
  for (i = first; i != last; ++i) {

    // Normals.

    // Test rule 1.
    if (!(*i).has_any_normal()) {
      bind_normals = G_OFF;
    } else if (bind_normals != G_OFF) {
      // Test rule 2.
      if ((*i).has_vertex_normal()) {
        bind_normals = G_PER_VERTEX;
      } else if (bind_normals != G_PER_VERTEX) {
        // Test rule 3.
        if ((*i).has_component_normal()) {
          bind_normals = G_PER_COMPONENT;
        } else if (bind_normals != G_PER_COMPONENT) {
          // Test rule 4.
          nassertr((*i).has_overall_normal(), 0);
          if (first_normal) {
            overall_normal = (*i).get_normal();
            first_normal = false;
          } else if ( !((*i).get_normal() == overall_normal)) {
            bind_normals = G_PER_PRIM;
          }
        }
      }
    }

    // Colors.

    // Test rule 1.
    if (!(*i).has_any_color()) {
      bind_colors = G_OFF;
    } else if (bind_colors != G_OFF) {
      // Test rule 2.
      if ((*i).has_vertex_color()) {
        bind_colors = G_PER_VERTEX;
      } else if (bind_colors != G_PER_VERTEX) {
        // Test rule 3.
        if ((*i).has_component_color()) {
          bind_colors = G_PER_COMPONENT;
        } else if (bind_colors != G_PER_COMPONENT) {
          // Test rule 4.
          nassertr((*i).has_overall_color(), 0);
          if (first_color) {
            overall_color = (*i).get_color();
            first_color = false;
          } else if ( !((*i).get_color() == overall_color)) {
            bind_colors = G_PER_PRIM;
          }
        }
      }
    }

    // Texcoords.

    // Test rule 1.
    if (!(*i).has_any_texcoord()) {
      bind_texcoords = G_OFF;
    }
  }

  // Determine the primitive type and build the lengths array, if needed.
  PTA_int lengths;
  bool want_lengths = false;
  int j;

  Geom *geom = NULL;
  BuilderPrimType type = (*first).get_type();

  switch (type) {
  case BPT_poly:
    geom = new GeomPolygon;
    want_lengths = true;
    break;

  case BPT_tristrip:
    geom = new GeomTristrip;
    want_lengths = true;
    break;

  case BPT_trifan:
    geom = new GeomTrifan;
    want_lengths = true;
    break;

  case BPT_line:
    geom = new GeomLine;
    break;

  case BPT_linestrip:
    geom = new GeomLinestrip;
    want_lengths = true;
    break;

  case BPT_point:
    geom = new GeomPoint;
    break;

  case BPT_tri:
    geom = new GeomTri;
    break;

  case BPT_quad:
    geom = new GeomQuad;
    break;

  default:
    builder_cat.fatal() << "Invalid primitive type.\n";
    abort();
  }

  if (geom == NULL) {
    builder_cat.error() << "Unsupported primitive type " << type << "\n";
    return 0;
  }

  // Count up the number of prims we're actually building.
  int num_prims = 0;
  for (i = first; i != last; ++i) {
    if ((*i).is_valid()) {
      num_prims++;
    }
  }

  if (num_prims==0) {
    builder_cat.error() << "All primitives were invalid!\n";
    return 0;
  }

  if (want_lengths) {
    lengths = PTA_int::empty_array(num_prims);
    j = 0;
    for (i = first; i != last; ++i) {
      if ((*i).is_valid()) {
        lengths[j++] = (*i).get_num_verts();
      }
    }
    nassertr(j == num_prims, 0);
  }

  // Now build up some arrays.
  PTA(VType) coords=PTA(VType)::empty_array(0);
  PTA(NType) normals=PTA(NType)::empty_array(0);
  PTA(TType) texcoords=PTA(TType)::empty_array(0);
  PTA(CType) colors=PTA(CType)::empty_array(0);

  int total_verts = 0;
  int total_components = 0;

  int v, num_verts;
  int c, num_components;
  for (i = first; i != last; ++i) {
    if ((*i).is_valid()) {
      num_verts = (*i).get_num_verts();
      total_verts += num_verts;
      for (v = 0; v < num_verts; v++) {
        coords.push_back((*i).get_vertex(v).get_coord());

        if (bind_normals == G_PER_VERTEX) {
          normals.push_back((*i).get_vertex(v).get_normal());
        }
        if (bind_texcoords == G_PER_VERTEX) {
          texcoords.push_back((*i).get_vertex(v).get_texcoord());
        }
        if (bind_colors == G_PER_VERTEX) {
          colors.push_back((*i).get_vertex(v).get_color());
        }
      }

      num_components = (*i).get_num_components();
      total_components += num_components;
      for (c = 0; c < num_components; c++) {
        if (bind_normals == G_PER_COMPONENT) {
          normals.push_back((*i).get_component(c).get_normal());
        }
        if (bind_colors == G_PER_COMPONENT) {
          colors.push_back((*i).get_component(c).get_color());
        }
      }

      if (bind_normals == G_PER_PRIM) {
        normals.push_back((*i).get_normal());
      }
      if (bind_colors == G_PER_PRIM) {
        colors.push_back((*i).get_color());
      }
    }
  }

  if (bind_normals == G_OVERALL) {
    normals.push_back(overall_normal);
  }
  if (bind_colors == G_OVERALL) {
    colors.push_back(overall_color);
  }

  // Now add all the stuff to our Geom.

  geom->set_num_prims(num_prims);

  if (lengths != (int *)NULL) {
    geom->set_lengths(lengths);
  }

  PrimType::fill_geom(geom, coords,
                      bind_normals, normals,
                      bind_texcoords, texcoords,
                      bind_colors, colors,
                      bucket, num_prims,
                      total_components, total_verts);

    /*
  if ((*first).has_pixel_size()) {
    // Again, we only have to test the first one in the list for a
    // pixel_size attribute.  If this one has it, then they all have
    // the same value.
    geom->setPntSize((*first).get_pixel_size());
    geom->setLineWidth((*first).get_pixel_size());
  }
    */

  //  geom->setDrawBin(bucket._drawBin);
  //  geom->setDrawOrder(bucket._drawOrder);

  Geom *new_geom = bucket.done_geom(geom);
  if (new_geom != (Geom *)NULL) {
    geom_node->add_geom(new_geom, bucket._state);
  }

  return 1;
}


/////////////////////////////////////////////////////////////////////
//       Class : PrimByType
// Description : An STL function object to sort primitives in order by
//               type.
////////////////////////////////////////////////////////////////////
template<class PrimType>
class PrimByType {
public:
  int operator () (const PrimType &p1, const PrimType &p2) const {
    return p1.get_type() < p2.get_type();
  }
};


////////////////////////////////////////////////////////////////////
//     Function: __mesh_and_build
//  Description: The implementation of mesh_and_build(), below.  This
//               extra function call is just to allow mesh_and_build()
//               to infer the PrimType (BuilderPrim or BuilderPrimI)
//               from the iterator's value type, and template on that.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class PrimType>
static int
__mesh_and_build(InputIterator first, InputIterator last,
                 BuilderBucket &bucket, GeomNode *geom_node,
                 PrimType *) {
  if (first==last) {
    return 0;
  }

  typedef pvector<PrimType> Prims;
  Prims prims;
  BuilderBucket *local_bucket = NULL;
  BuilderBucket *bucket_ptr = &bucket;

  if (bucket._mesh) {
    // Send all the prims through the mesher.  First, make a copy of
    // the bucket so the mesher can modify it if it wants.
    local_bucket = bucket.make_copy();
    bucket_ptr = local_bucket;
    MesherTempl<PrimType> mesher(local_bucket);

    for (InputIterator ii = first; ii != last; ++ii) {
      mesher.add_prim(*ii);
    }
    mesher.mesh();
    PrimType prim;
    prim = mesher.getPrim();
    while (prim.get_num_verts() > 0) {
      prims.push_back(prim);
      prim = mesher.getPrim();
    }

  } else {
    // Send the prims through without meshing.
    copy(first, last, back_inserter(prims));
  }

  // Now we have an array of prims which all share the same
  // properties, except possibly type.  Sort them by type and send
  // them to build_geoms.
  sort(prims.begin(), prims.end(), PrimByType<PrimType>());

  int count = 0;
  if (!prims.empty()) {
    TYPENAME Prims::iterator pi, last_pi;
    pi = prims.begin();
    last_pi = pi;
    for (++pi; pi != prims.end(); ++pi) {
      if ((*pi).get_type() != (*last_pi).get_type()) {
        count += build_geoms(last_pi, pi, *bucket_ptr, geom_node, (PrimType*)0);
        last_pi = pi;
      }
    }
    count += build_geoms(last_pi, pi, *bucket_ptr, geom_node, (PrimType*)0);
  }

  if (local_bucket!=NULL) {
    delete local_bucket;
  }

  // Finally, if the user so requested, create some visualization for
  // the normals.
#ifdef SUPPORT_SHOW_NORMALS
  if (bucket._show_normals) {
    BuilderNormalVisualizer bnv(bucket);
    for (InputIterator ii = first; ii != last; ++ii) {
      bnv.add_prim(*ii);
    }
    bnv.show_normals(geom_node);
  }
#endif

  return count;
}


////////////////////////////////////////////////////////////////////
//     Function: mesh_and_build
//  Description: Accepts a list of BuilderPrim or BuilderPrimI
//               objects, defined by the iterators first and list,
//               runs them through the mesher if specified by the
//               bucket, and builds them into the indicated GeomNode.
////////////////////////////////////////////////////////////////////
template<class InputIterator, class value_type>
int
mesh_and_build(InputIterator first, InputIterator last,
               BuilderBucket &bucket, GeomNode *geom_node,
               value_type *value_type_ptr) {
  return __mesh_and_build(first, last, bucket, geom_node, value_type_ptr);
}


////////////////////////////////////////////////////////////////////
//     Function: split
//  Description: Splits an STL list into two other lists, according to
//               the return value from pred.
////////////////////////////////////////////////////////////////////
template <class InputIterator, class OutputIterator, class Predicate>
OutputIterator split(InputIterator first, InputIterator last,
                     OutputIterator true_result, OutputIterator false_result,
                     Predicate pred) {
  while (first != last) {
    if (pred(*first)) {
      *true_result++ = *first++;
    } else {
      *false_result++ = *first++;
    }
  }
  return true_result;
}

