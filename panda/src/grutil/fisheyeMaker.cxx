/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fisheyeMaker.cxx
 * @author drose
 * @date 2005-10-03
 */

#include "fisheyeMaker.h"
#include "geomNode.h"
#include "geom.h"
#include "geomTristrips.h"
#include "geomVertexWriter.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "internalName.h"
#include "luse.h"
#include "cmath.h"
#include "mathNumbers.h"
#include "graphicsStateGuardian.h"
#include "displayRegion.h"

/**
 * Resets all the parameters to their initial defaults.
 */
void FisheyeMaker::
reset() {
  set_fov(360.0);
  _num_vertices = 1000;
  _square_inscribed = false;
  _square_radius = 1.0f;
  set_reflection(false);
}

/**
 * Specifies the field of view of the fisheye projection.  A sphere map will
 * have a 360-degree field of view (and this is the default).
 */
void FisheyeMaker::
set_fov(PN_stdfloat fov) {
  _fov = fov;
  _half_fov_rad =  deg_2_rad(_fov * 0.5f);
}


/**
 * Generates a GeomNode that renders the specified geometry.
 */
PT(PandaNode) FisheyeMaker::
generate() {
  // Get some system-imposed limits.
  int max_vertices_per_array = 100;
  int max_vertices_per_primitive = 10000;
  bool prefers_triangle_strips = true;

  /*
  GraphicsStateGuardian *global_gsg = GraphicsStateGuardian::get_global_gsg();
  if (global_gsg != (GraphicsStateGuardian *)NULL) {
    max_vertices_per_array = global_gsg->get_max_vertices_per_array();
    max_vertices_per_primitive = global_gsg->get_max_vertices_per_primitive();
    prefers_triangle_strips = global_gsg->prefers_triangle_strips();
  }
  */

  // We will generate a rose of radius 1, with vertices approximately evenly
  // distributed throughout.

  // Since we will have _num_vertices filling the circle, and the area of a
  // circle of radius 1 is A = pi*r^2 = pi, it follows that the number of
  // vertices per square unit is (_num_vertices  pi), and thus the number of
  // vertices per linear unit is the square root of that.
  PN_stdfloat vertices_per_unit = csqrt(_num_vertices / MathNumbers::pi_f);
  PN_stdfloat two_pi = 2.0f * MathNumbers::pi_f;

  // The rose will be made up of concentric rings, originating from the
  // center, to a radius of 1.0.
  int num_rings = (int)floor(vertices_per_unit + 0.5f);

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format
    (new GeomVertexArrayFormat
     (InternalName::get_vertex(), 3,
      Geom::NT_stdfloat, Geom::C_point,
      InternalName::get_texcoord(), 3,
      Geom::NT_stdfloat, Geom::C_texcoord));

  PT(GeomVertexData) vdata =
    new GeomVertexData(get_name(), format, Geom::UH_static);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  PT(Geom) geom = new Geom(vdata);
  PT(GeomPrimitive) tristrips = new GeomTristrips(Geom::UH_static);
  tristrips->set_shade_model(Geom::SM_uniform);

  PT(GeomNode) geom_node = new GeomNode(get_name());

  int last_ring_size = 3;
  int last_ring_vertex = 0;
  PN_stdfloat last_r = 1.0f / (PN_stdfloat)num_rings;

  // Make the first triangle.  We actually make a one-triangle strip, but that
  // seems more sensible than making a single isolated triangle.
  for (int vi = 0; vi < last_ring_size; ++vi) {
    add_vertex(vertex, texcoord, last_r,
               two_pi * (PN_stdfloat)vi / (PN_stdfloat)last_ring_size);
    tristrips->add_vertex(vi);
  }
  // Actually, we need to add one more degenerate triangle to make it an even-
  // length tristrip.
  tristrips->add_vertex(2);
  tristrips->close_primitive();

  // Now make all of the rings.
  for (int ri = 1; ri < num_rings; ++ri) {
    PN_stdfloat r = (PN_stdfloat)(ri + 1) / (PN_stdfloat)num_rings;

    // The circumference of a ring of radius r is 2*pi*r.
    PN_stdfloat c = two_pi * r;
    int ring_size = (int)floor(c * vertices_per_unit + 0.5f);

    // Each ring must either have exactly the same number of vertices as the
    // previous ring, or exactly double.
    if (ring_size < last_ring_size * 2) {
      // This one will be the same.
      ring_size = last_ring_size;
    } else {
      // This one will be double.
      ring_size = last_ring_size * 2;
    }

    if (vdata->get_num_rows() + ring_size > max_vertices_per_array) {
      // Too many vertices; we need to start a new VertexData.
      if (tristrips->get_num_vertices() != 0) {
        geom->add_primitive(tristrips);
      }
      if (geom->get_num_primitives() != 0) {
        if (prefers_triangle_strips) {
          geom_node->add_geom(geom);
        } else {
          geom_node->add_geom(geom->decompose());
        }
      }

      vdata = new GeomVertexData(get_name(), format, Geom::UH_static);
      vertex = GeomVertexWriter(vdata, InternalName::get_vertex());
      texcoord = GeomVertexWriter(vdata, InternalName::get_texcoord());
      geom = new Geom(vdata);
      tristrips = new GeomTristrips(Geom::UH_static);
      tristrips->set_shade_model(Geom::SM_uniform);

      // Now we need to re-make the previous ring in this VertexData.
      last_ring_vertex = 0;
      for (int vi = 0; vi < last_ring_size; ++vi) {
        add_vertex(vertex, texcoord, last_r,
                   two_pi * (PN_stdfloat)vi / (PN_stdfloat)last_ring_size);
      }
    }

    // Now make this ring.
    int ring_vertex = vdata->get_num_rows();
    for (int vi = 0; vi < ring_size; ++vi) {
      add_vertex(vertex, texcoord, r,
                 two_pi * (PN_stdfloat)vi / (PN_stdfloat)ring_size);
    }

    // Now draw the triangle strip to connect the rings.
    if (ring_size == last_ring_size) {
      // Exactly the same size ring.  This one is easy.
      if ((ring_size + 1) * 2 > max_vertices_per_primitive) {
        // Actually, we need to subdivide the ring to fit within the GSG's
        // advertised limits.
        int piece_size = max_vertices_per_primitive / 2 - 1;
        int vi = 0;
        while (vi < ring_size) {
          int piece_end = std::min(ring_size + 1, piece_size + 1 + vi);
          for (int pi = vi; pi < piece_end; ++pi) {
            tristrips->add_vertex(last_ring_vertex + pi % last_ring_size);
            tristrips->add_vertex(ring_vertex + pi % ring_size);
          }
          tristrips->close_primitive();
          vi += piece_size;
        }

      } else {
        // We can fit the entire ring.
        if (tristrips->get_num_vertices() > 0 &&
            tristrips->get_num_vertices() + ring_size * 2 > max_vertices_per_primitive) {
          geom->add_primitive(tristrips);
          tristrips = new GeomTristrips(Geom::UH_static);
          tristrips->set_shade_model(Geom::SM_uniform);
        }
        for (int vi = 0; vi < ring_size; ++vi) {
          tristrips->add_vertex(last_ring_vertex + vi);
          tristrips->add_vertex(ring_vertex + vi);
        }
        tristrips->add_vertex(last_ring_vertex);
        tristrips->add_vertex(ring_vertex);
        tristrips->close_primitive();
      }

    } else {
      // Exactly double size ring.  This is harder; we can't make a single
      // tristrip that goes all the way around the ring.  Instead, we'll make
      // an alternating series of four-triangle strips and two-triangle strips
      // around the ring.
      int vi = 0;
      while (vi < last_ring_size) {
        if (tristrips->get_num_vertices() + 10 > max_vertices_per_primitive) {
          geom->add_primitive(tristrips);
          tristrips = new GeomTristrips(Geom::UH_static);
          tristrips->set_shade_model(Geom::SM_uniform);
        }
        tristrips->add_vertex(ring_vertex + (vi * 2 + 1) % ring_size);
        tristrips->add_vertex(ring_vertex + (vi * 2 + 2) % ring_size);
        tristrips->add_vertex(last_ring_vertex + (vi + 1) % last_ring_size);
        tristrips->add_vertex(ring_vertex + (vi * 2 + 3) % ring_size);
        tristrips->add_vertex(last_ring_vertex + (vi + 2) % last_ring_size);
        tristrips->add_vertex(ring_vertex + (vi * 2 + 4) % ring_size);
        tristrips->close_primitive();

        tristrips->add_vertex(ring_vertex + (vi * 2 + 4) % ring_size);
        tristrips->add_vertex(ring_vertex + (vi * 2 + 5) % ring_size);
        tristrips->add_vertex(last_ring_vertex + (vi + 2) % last_ring_size);
        tristrips->add_vertex(last_ring_vertex + (vi + 3) % last_ring_size);
        tristrips->close_primitive();

        vi += 2;
      }
    }

    last_ring_size = ring_size;
    last_ring_vertex = ring_vertex;
    last_r = r;
  }

  if (_square_inscribed) {
    // Make one more "ring", which extends out to the edges of a squre.
    int ring_size = last_ring_size;

    if (vdata->get_num_rows() + ring_size > max_vertices_per_array) {
      // Too many vertices; we need to start a new VertexData.
      if (tristrips->get_num_vertices() != 0) {
        geom->add_primitive(tristrips);
      }
      if (geom->get_num_primitives() != 0) {
        if (prefers_triangle_strips) {
          geom_node->add_geom(geom);
        } else {
          geom_node->add_geom(geom->decompose());
        }
      }

      vdata = new GeomVertexData(get_name(), format, Geom::UH_static);
      vertex = GeomVertexWriter(vdata, InternalName::get_vertex());
      texcoord = GeomVertexWriter(vdata, InternalName::get_texcoord());
      geom = new Geom(vdata);
      tristrips = new GeomTristrips(Geom::UH_static);
      tristrips->set_shade_model(Geom::SM_uniform);

      // Now we need to re-make the previous ring in this VertexData.
      last_ring_vertex = 0;
      for (int vi = 0; vi < last_ring_size; ++vi) {
        add_vertex(vertex, texcoord, last_r,
                   two_pi * (PN_stdfloat)vi / (PN_stdfloat)last_ring_size);
      }
    }

    // Now make this ring.
    int ring_vertex = vdata->get_num_rows();
    for (int vi = 0; vi < ring_size; ++vi) {
      add_square_vertex(vertex, texcoord,
                        two_pi * (PN_stdfloat)vi / (PN_stdfloat)ring_size);
    }

    // Now draw the triangle strip to connect the rings.
    if ((ring_size + 1) * 2 > max_vertices_per_primitive) {
      // Actually, we need to subdivide the ring to fit within the GSG's
      // advertised limits.
      int piece_size = max_vertices_per_primitive / 2 - 1;
      int vi = 0;
      while (vi < ring_size) {
        int piece_end = std::min(ring_size + 1, piece_size + 1 + vi);
        for (int pi = vi; pi < piece_end; ++pi) {
          tristrips->add_vertex(last_ring_vertex + pi % last_ring_size);
          tristrips->add_vertex(ring_vertex + pi % ring_size);
        }
        tristrips->close_primitive();
        vi += piece_size;
      }

    } else {
      // We can fit the entire ring.
      if (tristrips->get_num_vertices() > 0 &&
          tristrips->get_num_vertices() + ring_size * 2 > max_vertices_per_primitive) {
        geom->add_primitive(tristrips);
        tristrips = new GeomTristrips(Geom::UH_static);
        tristrips->set_shade_model(Geom::SM_uniform);
      }
      for (int vi = 0; vi < ring_size; ++vi) {
        tristrips->add_vertex(last_ring_vertex + vi);
        tristrips->add_vertex(ring_vertex + vi);
      }
      tristrips->add_vertex(last_ring_vertex);
      tristrips->add_vertex(ring_vertex);
      tristrips->close_primitive();
    }
  }

  if (tristrips->get_num_vertices() != 0) {
    geom->add_primitive(tristrips);
  }
  if (geom->get_num_primitives() != 0) {
    if (prefers_triangle_strips) {
      geom_node->add_geom(geom);
    } else {
      geom_node->add_geom(geom->decompose());
    }
  }

  return geom_node;
}

/**
 * Given a point defined by a radius and an angle in radians, compute the 2-d
 * coordinates for the vertex as well as the 3-d texture coordinates, and add
 * both to the VertexData.
 */
void FisheyeMaker::
add_vertex(GeomVertexWriter &vertex, GeomVertexWriter &texcoord,
           PN_stdfloat r, PN_stdfloat a) {
  PN_stdfloat sina, cosa;
  csincos(a, &sina, &cosa);

  // The 2-d point is just a point r units from the center of the circle.
  LPoint3 point(r * cosa, 0.0f, r * sina);
  vertex.add_data3(point);

  // The 3-d point is the same thing, bent through the third dimension around
  // the surface of a sphere to the point in the back.
  PN_stdfloat b = r * _half_fov_rad;
  if (b >= MathNumbers::pi_f) {
    // Special case: we want to stop at the back pole, not continue around it.
    texcoord.add_data3(0, _reflect, 0);

  } else {
    PN_stdfloat sinb, cosb;
    csincos(b, &sinb, &cosb);
    LPoint3 tc(sinb * cosa, cosb * _reflect, sinb * sina);
    texcoord.add_data3(tc);
  }
}

/**
 * Similar to add_vertex(), but it draws the vertex all the way out to the
 * edge of the square we are inscribed within, and the texture coordinate is
 * always the back pole.
 *
 * This is just for the purpose of drawing the inscribing square.
 */
void FisheyeMaker::
add_square_vertex(GeomVertexWriter &vertex, GeomVertexWriter &texcoord,
                  PN_stdfloat a) {
  PN_stdfloat sina, cosa;
  csincos(a, &sina, &cosa);

  // Extend the 2-d point to the edge of the square of the indicated size.
  if (cabs(sina) > cabs(cosa)) {
    PN_stdfloat y = (sina > 0.0f) ? _square_radius : -_square_radius;
    PN_stdfloat x = y * cosa / sina;
    LPoint3 point(x, 0.0f, y);
    vertex.add_data3(point);

  } else {
    PN_stdfloat x = (cosa > 0.0f) ? _square_radius : -_square_radius;
    PN_stdfloat y = x * sina / cosa;
    LPoint3 point(x, 0.0f, y);
    vertex.add_data3(point);
  }

  texcoord.add_data3(0, _reflect, 0);
}
