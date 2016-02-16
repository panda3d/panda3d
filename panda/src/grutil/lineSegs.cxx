/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lineSegs.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "lineSegs.h"
#include "renderState.h"
#include "renderModeAttrib.h"
#include "geom.h"
#include "geomLinestrips.h"
#include "geomPoints.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "colorAttrib.h"

/**
 * Constructs a LineSegs object, which can be used to create any number of
 * disconnected lines or points of various thicknesses and colors through the
 * visible scene.  After creating the object, call move_to() and draw_to()
 * repeatedly to describe the path, then call create() to create a GeomNode
 * which will render the described path.
 */
LineSegs::
LineSegs(const string &name) : Namable(name) {
  _color.set(1.0f, 1.0f, 1.0f, 1.0f);
  _thick = 1.0f;
}


/**

 */
LineSegs::
~LineSegs() {
}


/**
 * Removes any lines in progress and resets to the initial empty state.
 */
void LineSegs::
reset() {
  _list.clear();
}


/**
 * Moves the pen to the given point without drawing a line.  When followed by
 * draw_to(), this marks the first point of a line segment; when followed by
 * move_to() or create(), this creates a single point.
 */
void LineSegs::
move_to(const LVecBase3 &v) {
  // We create a new SegmentList with the initial point in it.
  SegmentList segs;
  segs.push_back(Point(v, _color));

  // And add this list to the list of segments.
  _list.push_back(segs);
}

/**
 * Draws a line segment from the pen's last position (the last call to move_to
 * or draw_to) to the indicated point.  move_to() and draw_to() only update
 * tables; the actual drawing is performed when create() is called.
 */
void LineSegs::
draw_to(const LVecBase3 &v) {
  if (_list.empty()) {
    // Let our first call to draw_to() be an implicit move_to().
    move_to(v);

  } else {
    // Get the current SegmentList, which was the last one we added to
    // the LineList.
    SegmentList &segs = _list.back();

    // Add the new point.
    segs.push_back(Point(v, _color));
  }
}

/**
 * Returns true if move_to() or draw_to() have not been called since the last
 * reset() or create(), false otherwise.
 */
bool LineSegs::
is_empty() {
  return _list.empty();
}

/**
 * Returns the nth point or vertex of the line segment sequence generated by the
 * last call to create().  The first move_to() generates vertex 0; subsequent
 * move_to() and draw_to() calls generate consecutively higher vertex numbers.
 */
LVertex LineSegs::
get_vertex(int n) const {
  nassertr(_created_data != (GeomVertexData *)NULL, LVertex::zero());
  GeomVertexReader vertex(_created_data, InternalName::get_vertex());
  vertex.set_row_unsafe(n);
  return vertex.get_data3();
}

/**
 * Moves the nth point or vertex of the line segment sequence generated by the
 * last call to create().  The first move_to() generates vertex 0; subsequent
 * move_to() and draw_to() calls generate consecutively higher vertex numbers.
 */
void LineSegs::
set_vertex(int n, const LVertex &vert) {
  nassertv(_created_data != (GeomVertexData *)NULL);
  GeomVertexWriter vertex(_created_data, InternalName::get_vertex());
  vertex.set_row_unsafe(n);
  vertex.set_data3(vert);
}

/**
 * Returns the color of the nth point or vertex.
 */
LColor LineSegs::
get_vertex_color(int n) const {
  nassertr(_created_data != (GeomVertexData *)NULL, LColor::zero());
  GeomVertexReader color(_created_data, InternalName::get_color());
  color.set_row_unsafe(n);
  return color.get_data4();
}

/**
 * Changes the vertex color of the nth point or vertex.  See set_vertex().
 */
void LineSegs::
set_vertex_color(int n, const LColor &c) {
  nassertv(_created_data != (GeomVertexData *)NULL);
  GeomVertexWriter color(_created_data, InternalName::get_color());
  color.set_row_unsafe(n);
  color.set_data4(c);
}

/**
 * Returns the pen's current position.  The next call to draw_to() will draw a
 * line segment from this point.
 */
const LVertex &LineSegs::
get_current_position() {
  if (_list.empty()) {
    // Our pen isn't anywhere.  We'll put it somewhere.
    move_to(LVertex(0.0f, 0.0f, 0.0f));
  }

  return _list.back().back()._point;
}

/**
 * Appends to an existing GeomNode a new Geom that will render the series of
 * line segments and points described via calls to move_to() and draw_to().  The
 * lines and points are created with the color and thickness established by
 * calls to set_color() and set_thick().  If dynamic is true, the line segments
 * will be created with the dynamic Geom setting, optimizing them for runtime
 * vertex animation.
 */
GeomNode *LineSegs::
create(GeomNode *previous, bool dynamic) {
  if (!_list.empty()) {
    CPT(RenderAttrib) thick = RenderModeAttrib::make(RenderModeAttrib::M_unchanged, _thick);
    CPT(RenderAttrib) vtxcolor = ColorAttrib::make_vertex();
    CPT(RenderState) state = RenderState::make(thick, vtxcolor);

    _created_data = new GeomVertexData
      ("lineSegs", GeomVertexFormat::get_v3cp(),
       dynamic ? Geom::UH_dynamic : Geom::UH_static);
    GeomVertexWriter vertex(_created_data, InternalName::get_vertex());
    GeomVertexWriter color(_created_data, InternalName::get_color());

    PT(GeomLinestrips) lines = new GeomLinestrips(Geom::UH_static);
    PT(GeomPoints) points = new GeomPoints(Geom::UH_static);

    int v = 0;
    LineList::const_iterator ll;
    SegmentList::const_iterator sl;

    for (ll = _list.begin(); ll != _list.end(); ll++) {
      const SegmentList &segs = (*ll);

      if (segs.size() < 2) {
        // A segment of length 1 is just a point.
        for (sl = segs.begin(); sl != segs.end(); sl++) {
          points->add_vertex(v);
          vertex.add_data3((*sl)._point);
          color.add_data4((*sl)._color);
          v++;
        }
        points->close_primitive();

      } else {
        // A segment of length 2 or more is a line segment or
        // segments.
        for (sl = segs.begin(); sl != segs.end(); sl++) {
          lines->add_vertex(v);
          vertex.add_data3((*sl)._point);
          color.add_data4((*sl)._color);
          v++;
        }
        lines->close_primitive();
      }
    }

    if (lines->get_num_vertices() != 0) {
      PT(Geom) geom = new Geom(_created_data);
      geom->add_primitive(lines);
      previous->add_geom(geom, state);
    }
    if (points->get_num_vertices() != 0) {
      PT(Geom) geom = new Geom(_created_data);
      geom->add_primitive(points);
      previous->add_geom(geom, state);
    }

    // And reset for next time.
    reset();
  }

  return previous;
}
