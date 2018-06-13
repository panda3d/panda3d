/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textGlyph.cxx
 * @author drose
 * @date 2002-02-08
 */

#include "textGlyph.h"
#include "geomTextGlyph.h"
#include "geomTriangles.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"

using std::max;
using std::min;

TypeHandle TextGlyph::_type_handle;

/**
 *
 */
TextGlyph::
~TextGlyph() {
}

/**
 * Returns true if this glyph represents invisible whitespace, or false if it
 * corresponds to some visible character.
 */
bool TextGlyph::
is_whitespace() const {
  // In a static font, there is no explicit glyph for whitespace, so all
  // glyphs are non-whitespace.
  return false;
}

/**
 * Returns a Geom that renders the particular glyph.  It will be generated if
 * necessary.
 *
 * This method will always return a copy of the Geom, so the caller is free to
 * modify it.
 */
PT(Geom) TextGlyph::
get_geom(Geom::UsageHint usage_hint) const {
  if (_geom.is_null()) {
    // Maybe we have yet to generate a suitable Geom.
    if (_has_quad) {
      ((TextGlyph *)this)->make_quad_geom();
      if (_geom.is_null()) {
        return nullptr;
      }
    } else {
      // Nope.
      return nullptr;
    }
  }

  // We always return a copy of the geom.  That will allow the caller to
  // modify its vertices without fear of stomping on other copies.  It is also
  // important that we store a reference to this glyph on the Geom, since the
  // DynamicTextFont relies on counting references to determine whether a
  // glyph is no longer used.
  PT(Geom) new_geom = new GeomTextGlyph(*_geom, this);
  new_geom->set_usage_hint(usage_hint);
  const GeomVertexData *vdata = new_geom->get_vertex_data();
  nassertr(vdata != nullptr, new_geom);
  if (vdata->get_usage_hint() != usage_hint) {
    new_geom->modify_vertex_data()->set_usage_hint(usage_hint);
  }
  return new_geom;
}

/**
 * Expands min_point and max_point to include all of the vertices in the
 * glyph, if any.  found_any is set true if any points are found.  It is the
 * caller's responsibility to initialize min_point, max_point, and found_any
 * before calling this function.
 */
void TextGlyph::
calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                  bool &found_any, Thread *current_thread) const {
  if (_has_quad) {
    found_any = true;
    min_point.set(_quad_dimensions[0], 0, _quad_dimensions[1]);
    max_point.set(_quad_dimensions[2], 0, _quad_dimensions[3]);

  } else if (!_geom.is_null()) {
    _geom->calc_tight_bounds(min_point, max_point, found_any, current_thread);
  }
}

/**
 * Sets the glyph using the given quad parameters.  Any Geom assigned will be
 * cleared.  The order of the components is left, bottom, right, top.
 */
void TextGlyph::
set_quad(const LVecBase4 &dimensions, const LVecBase4 &texcoords,
         const RenderState *state) {
  _geom.clear();

  _quad_dimensions = dimensions;
  _quad_texcoords = texcoords;
  _has_quad = true;
  _state = state;
}

/**
 * Sets the geom from a pre-built Geom object.  Any quad parameters assigned
 * will be cleared.
 */
void TextGlyph::
set_geom(GeomVertexData *vdata, GeomPrimitive *prim,
         const RenderState *state) {
  PT(Geom) geom = new GeomTextGlyph(this, vdata);
  geom->add_primitive(prim);
  _geom = geom;

  _has_quad = false;
  _state = state;
}

/**
 * Checks if the geom that was passed in is actually a quad, and if so, sets
 * the appropriate quad parameters.
 *
 * This is useful when loading static text fonts, so that they can still
 * benefit from the fast text assembly that is done for quads.
 */
void TextGlyph::
check_quad_geom() {
  // Currently it looks for rather specific signs that this glyph has been
  // generated using egg-mkfont.  For now, this is fine.
  CPT(GeomVertexData) vdata = _geom->get_vertex_data();
  if (vdata->get_num_rows() != 4) {
    return;
  }

  // Does it appear to have a single primitive with two triangles?
  if (_geom->get_num_primitives() != 1) {
    return;
  }
  CPT(GeomPrimitive) prim = _geom->get_primitive(0);
  if (!prim->is_indexed() ||
      prim->get_primitive_type() != GeomPrimitive::PT_polygons) {
    return;
  }
  if (!prim->is_of_type(GeomTriangles::get_class_type())) {
    prim = prim->decompose();
  }
  if (prim->get_num_vertices() != 6) {
    return;
  }

  // Check that it has a vertex column and optionally a texcoord column.
  CPT(GeomVertexFormat) format = vdata->get_format();
  if (format->get_num_columns() > 2 ||
      !format->has_column(InternalName::get_vertex())) {
    return;
  }
  if (format->has_column(InternalName::get_texcoord()) != (format->get_num_columns() == 2)) {
    // The second column, if there is one, is not a texcoord.
    return;
  }

  // Check that the vertices are arranged in a square.
  GeomVertexReader vertex(vdata, InternalName::get_vertex());
  LVecBase3 v = vertex.get_data3();
  if (!IS_NEARLY_ZERO(v[1])) {
    return;
  }
  PN_stdfloat minx = v[0];
  PN_stdfloat maxx = v[0];
  PN_stdfloat miny = v[2];
  PN_stdfloat maxy = v[2];

  for (int i = 0; i < 3; ++i) {
    v = vertex.get_data3();
    if (!IS_NEARLY_ZERO(v[1])) {
      return;
    }
    if (!IS_NEARLY_EQUAL(v[0], minx) && !IS_NEARLY_EQUAL(v[0], maxx)) {
      if (!IS_NEARLY_EQUAL(minx, maxx)) {
        return;
      }
      minx = min(v[0], minx);
      maxx = max(v[0], maxx);
    }
    if (!IS_NEARLY_EQUAL(v[2], miny) && !IS_NEARLY_EQUAL(v[2], maxy)) {
      if (!IS_NEARLY_EQUAL(miny, maxy)) {
        return;
      }
      miny = min(v[2], miny);
      maxy = max(v[2], maxy);
    }
  }

  PN_stdfloat minu = 0;
  PN_stdfloat maxu = 0;
  PN_stdfloat minv = 0;
  PN_stdfloat maxv = 0;

  // Same for the texcoord data.
  if (format->has_column(InternalName::get_texcoord())) {
    GeomVertexReader texcoord(vdata, InternalName::get_texcoord());
    LVecBase2 tc = texcoord.get_data2();
    minu = tc[0];
    maxu = tc[0];
    minv = tc[1];
    maxv = tc[1];

    for (int i = 0; i < 3; ++i) {
      tc = texcoord.get_data2();
      if (!IS_NEARLY_EQUAL(tc[0], minu) && !IS_NEARLY_EQUAL(tc[0], maxu)) {
        if (!IS_NEARLY_EQUAL(minu, maxu)) {
          return;
        }
        minu = min(tc[0], minu);
        maxu = max(tc[0], maxu);
      }
      if (!IS_NEARLY_EQUAL(tc[1], minv) && !IS_NEARLY_EQUAL(tc[1], maxv)) {
        if (!IS_NEARLY_EQUAL(minv, maxv)) {
          return;
        }
        minv = min(tc[1], minv);
        maxv = max(tc[1], maxv);
      }
    }
  }

  _quad_dimensions.set(minx, miny, maxx, maxy);
  _quad_texcoords.set(minu, minv, maxu, maxv);
  _has_quad = true;
}

/**
 * Generates a Geom representing this text glyph, if at all possible.
 */
void TextGlyph::
make_quad_geom() {
  // The default implementation is to generate a Geom based on the get_quad()
  // implementation, if any.
  LVecBase4 dimensions, uvs;
  if (!get_quad(dimensions, uvs)) {
    return;
  }

  // Create a corresponding triangle pair.  We use a pair of indexed triangles
  // rather than a single triangle strip, to avoid the bad vertex duplication
  // behavior with lots of two-triangle strips.
  PT(GeomVertexData) vdata = new GeomVertexData
    (std::string(), GeomVertexFormat::get_v3t2(), Geom::UH_static);
  vdata->unclean_set_num_rows(4);

  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

  {
    GeomVertexWriter vertex(vdata, InternalName::get_vertex());
    GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

    PT(GeomVertexArrayData) indices = tris->modify_vertices();
    indices->unclean_set_num_rows(6);

    GeomVertexWriter index(indices, 0);

    vertex.set_data3(dimensions[0], 0, dimensions[3]);
    vertex.set_data3(dimensions[0], 0, dimensions[1]);
    vertex.set_data3(dimensions[2], 0, dimensions[3]);
    vertex.set_data3(dimensions[2], 0, dimensions[1]);

    texcoord.set_data2(uvs[0], uvs[3]);
    texcoord.set_data2(uvs[0], uvs[1]);
    texcoord.set_data2(uvs[2], uvs[3]);
    texcoord.set_data2(uvs[2], uvs[1]);

    index.set_data1i(0);
    index.set_data1i(1);
    index.set_data1i(2);
    index.set_data1i(2);
    index.set_data1i(1);
    index.set_data1i(3);
  }

  // We create a regular Geom here, not a GeomTextGlyph, since doing so would
  // create a circular reference.  When the get_geom method makes a copy, it
  // will add in a pointer to this text glyph.
  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(tris);
  _geom = geom;
}
