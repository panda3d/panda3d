// Filename: lineSegs.h
// Created by:  drose (24May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef LINESEGS_H
#define LINESEGS_H

#include <pandabase.h>

#include <luse.h>
#include <geom.h>
#include <geomPoint.h>
#include <geomLine.h>
#include <geomLinestrip.h>
#include <geomNode.h>

#include <vector>

////////////////////////////////////////////////////////////////////
// 	 Class : LineSegs
// Description : Encapsulates creation of a series of connected or
//               disconnected line segments or points, for drawing
//               paths or rays.  This class doesn't attempt to be the
//               smartest it could possibly be; it's intended
//               primarily as a visualization and editing tool.
////////////////////////////////////////////////////////////////////
class LineSegs {
PUBLISHED:
  LineSegs();
  ~LineSegs();

  void reset();
  INLINE void set_color(float r, float g, float b, float a = 1.0);
  INLINE void set_color(const Colorf &color);
  INLINE void set_thickness(float thick);

  INLINE void move_to(float x, float y, float z);
  void move_to(const Vertexf &v);

  INLINE void draw_to(float x, float y, float z);
  void draw_to(const Vertexf &v);

  const Vertexf &get_current_position();
  bool is_empty();

  INLINE GeomNode *create(bool frame_accurate = false);
  GeomNode *create(GeomNode *previous, bool frame_accurate = false);

  // Functions to move the line vertices after they have been created.
  INLINE int get_num_vertices() const;

  INLINE Vertexf get_vertex(int vertex) const;
  INLINE void set_vertex(int vertex, const Vertexf &vert);
  INLINE void set_vertex(int vertex, float x, float y, float z);

  INLINE Colorf get_vertex_color(int vertex) const;
  INLINE void set_vertex_color(int vertex, const Colorf &color);
  INLINE void set_vertex_color(int vertex, float r, float g, float b, float a = 1.0);

private:
  class Point {
  public:
    INLINE Point();
    INLINE Point(const Vertexf &point, const Colorf &color);
    INLINE Point(const Point &copy);
    INLINE void operator = (const Point &copy);

    Vertexf _point;
    Colorf _color;
  };

  typedef vector<Point> SegmentList;
  typedef vector<SegmentList> LineList;

  LineList _list;
  Colorf _color;
  float _thick;

  PTA_Vertexf _created_verts;
  PTA_Colorf _created_colors;
};

#include "lineSegs.I"

#endif
