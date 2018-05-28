/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file heightfieldTesselator.cxx
 * @author jyelon
 * @date 2006-07-17
 */

#include "heightfieldTesselator.h"
#include "geomNode.h"
#include "transformState.h"
#include "sceneGraphReducer.h"
#include "lvector3.h"

/**
 * Makes sure that the heightfield is a grayscale image of valid dimensions.
 * If necessary, adds a band of zeros onto two sides of the heightfield, so as
 * to make the size of the heightfield a multiple of the given size plus one.
 */
void HeightfieldTesselator::
fix_heightfield(int size) {

  // Calculate the padded size of the heightfield.
  int xsize = _heightfield.get_x_size();
  int ysize = _heightfield.get_y_size();
  int xcells = (xsize + size - 2) / size;
  int ycells = (ysize + size - 2) / size;
  int xpadded = xcells * size + 1;
  int ypadded = ycells * size + 1;

  // If the heightfield is already in good shape, done.
  if ((xpadded == _heightfield.get_x_size()) &&
      (ypadded == _heightfield.get_y_size()) &&
      (_heightfield.is_grayscale())) {
    return;
  }

  // Pad the heightfield, and convert to grey.
  PNMImage unfixed(_heightfield);
  _heightfield.clear(xpadded, ypadded, 1,
                     unfixed.get_maxval(),
                     unfixed.get_type());
  for (int y = 0; y < ysize; y++) {
    for (int x = 0; x < xsize; x++) {
      _heightfield.set_gray_val(x, y, unfixed.get_gray_val(x, y));
    }
  }
  for (int y = ysize; y < ypadded; y++) {
    for (int x = xsize; x < xpadded; x++) {
      _heightfield.set_gray_val(x, y, 0);
    }
  }
}

/**
 * Fetches the elevation at (x,y), where the input coordinate is specified in
 * pixels.  This ignores the current tesselation level and instead provides an
 * accurate number.  Linear blending is used for non-integral coordinates.
 */
double HeightfieldTesselator::
get_elevation(double x, double y) {
  int scale = 7;
  int size = 1 << scale;
  fix_heightfield(size);
  int xlo = (int)x;
  int ylo = (int)y;
  if (xlo < 0) xlo = 0;
  if (ylo < 0) ylo = 0;
  if (xlo > _heightfield.get_x_size()-2)
    xlo = _heightfield.get_x_size()-2;
  if (ylo > _heightfield.get_y_size()-2)
    ylo = _heightfield.get_y_size()-2;
  int xhi = xlo+1;
  int yhi = ylo+1;
  double xoffs = x - xlo;
  double yoffs = y - ylo;
  double grayxlyl = _heightfield.get_gray(xlo,ylo);
  double grayxhyl = _heightfield.get_gray(xhi,ylo);
  double grayxlyh = _heightfield.get_gray(xlo,yhi);
  double grayxhyh = _heightfield.get_gray(xhi,yhi);
  double lerpyl = grayxhyl * xoffs + grayxlyl * (1.0 - xoffs);
  double lerpyh = grayxhyh * xoffs + grayxlyh * (1.0 - xoffs);
  return lerpyh * yoffs + lerpyl * (1.0 - yoffs);
}

/**
 * Fetches the vertex at (x,y), or if the vertex does not exist, creates it.
 */
int HeightfieldTesselator::
get_vertex(int x, int y) {
  int xsize = _heightfield.get_x_size();
  int vtx = _vertex_index[x+y*xsize];
  if (vtx >= 0) {
    return vtx;
  }
  int nx = x-1;
  int px = x+1;
  int ny = y-1;
  int py = y+1;
  if (nx < 0) nx++;
  if (ny < 0) ny++;
  if (px >= _heightfield.get_x_size()) px--;
  if (py >= _heightfield.get_y_size()) py--;
  double drx = _heightfield.get_gray(px,y) - _heightfield.get_gray(nx,y);
  double dry = _heightfield.get_gray(x,py) - _heightfield.get_gray(x,ny);
  LVector3 normal(drx * _vertical_scale * 0.5, dry * _vertical_scale * 0.5, _horizontal_scale);
  normal.normalize();
  double z = _heightfield.get_gray(x,y);
  _vertex_writer->add_data3(x*_horizontal_scale,-y*_horizontal_scale,z*_vertical_scale);
  _normal_writer->add_data3(normal);
  vtx = _next_index++;
  _dirty_vertices[vtx] = x+y*xsize;
  _vertex_index[x+y*xsize] = vtx;
  return vtx;
}


/**
 * Generates a tree of nodes that represents the heightfield.  This can be
 * reparented into the scene.
 */
NodePath HeightfieldTesselator::
generate() {
  int scale = 7;
  int size = 1 << scale;
  fix_heightfield(size);
  int xsize = _heightfield.get_x_size();
  int ysize = _heightfield.get_y_size();
  int xcells = (xsize + size - 2) / size;
  int ycells = (ysize + size - 2) / size;

  _vertex_index = new int[xsize * ysize];
  _dirty_vertices = new int[xsize * ysize];
  _triangle_totals = new int[xsize * ysize];
  for (int y=0; y<ysize; y++) {
    for (int x=0; x<xsize; x++) {
      _vertex_index[y*xsize+x] = -1;
    }
  }

  if (!_radii_calculated) {
    int saved_focal_x = _focal_x;
    int saved_focal_y = _focal_y;

    _focal_x = _heightfield.get_x_size() >> 1;
    _focal_y = _heightfield.get_y_size() >> 1;

    calculate_radii(scale);

    _focal_x = saved_focal_x;
    _focal_y = saved_focal_y;

    _radii_calculated = true;
  }

  PT(PandaNode) result = new PandaNode(get_name());
  NodePath root(result);

  int total = 0;
  for (int y=0; y<ycells; y++) {
    for (int x=0; x<xcells; x++) {
      total += count_triangles(scale,x*size,y*size);
    }
  }
  for (int y=0; y<ycells; y++) {
    for (int x=0; x<xcells; x++) {
      generate_square(root,scale,x*size,y*size,true);
    }
  }
  delete[] _vertex_index;
  delete[] _dirty_vertices;
  delete[] _triangle_totals;
  _vertex_index =nullptr;
  _dirty_vertices =nullptr;
  _triangle_totals =nullptr;

  return root;
}

/**
 * Sets the radii appropriately to achieve the desired polygon count.  This is
 * achieved by binary search.
 */
void HeightfieldTesselator::
calculate_radii(int scale) {
  int size = 1 << scale;
  int xsize = _heightfield.get_x_size();
  int ysize = _heightfield.get_y_size();
  int xcells = (xsize + size - 2) / size;
  int ycells = (ysize + size - 2) / size;

  double lo = 5.0;
  double hi = _heightfield.get_x_size() + _heightfield.get_y_size();
  while (1) {
    double mid = (lo + hi) * 0.5;
    for (int i=0; i<16; i++) {
      _radii[i] = (int)(mid * (1<<i));
    }
    int total = 0;
    for (int y=0; y<ycells; y++) {
      for (int x=0; x<xcells; x++) {
        total += count_triangles(scale,x*size,y*size);
      }
    }
    if (total > _poly_count) {
      hi = mid;
    } else {
      lo = mid;
    }
    if (hi - lo < 1.0) {
      break;
    }
  }
  double mid = (lo + hi) * 0.5;
  for (int i=0; i<16; i++) {
    _radii[i] = (int)(mid * (1<<i));
  }
}

/**
 * Adds a square region to the current geom.  This relies on the following
 * preconditions:
 *
 * 1. A square of scale N can be adjacent to a square of scale N or scale N-1,
 * but not scale N-2 or smaller.
 *
 * 2. A square of scale N can be adjacent to at most one square of scale N-1.
 *
 * Precondition 1 is assured by spacing out the detail radii sufficiently.
 * Precondition 2 is assured by using rectangular detail radii.
 *
 * I may someday rewrite this code to eliminate precondition 2, to allow
 * circular detail radii.
 */
void HeightfieldTesselator::
generate_square(NodePath root, int scale, int x, int y, bool forceclose) {
/*
 * There are nine possible vertices in the square, which are labeled as
 * follows: G--H--I |     | D  E  F |     | A--B--C
 */

  int size = 1<<scale;
  int hsize = size>>1;

#define POINTA get_vertex(x      ,y)
#define POINTB get_vertex(x+hsize,y)
#define POINTC get_vertex(x+size ,y)
#define POINTD get_vertex(x      ,y+hsize)
#define POINTE get_vertex(x+hsize,y+hsize)
#define POINTF get_vertex(x+size ,y+hsize)
#define POINTG get_vertex(x      ,y+size)
#define POINTH get_vertex(x+hsize,y+size)
#define POINTI get_vertex(x+size ,y+size)

  if (_triangles == nullptr) {
    open_geom();
  }
  if (subdivide(scale, x, y)) {
    int xc = x+(size>>1);
    int yc = y+(size>>1);
    if (_triangle_totals[yc*_heightfield.get_x_size()+xc] > _max_triangles) {
      if (_next_index) close_geom(root);
      NodePath subroot = root.attach_new_node(get_name() + " interior");
      generate_square(subroot, scale-1, x,  y , true);
      generate_square(subroot, scale-1, xc, y , true);
      generate_square(subroot, scale-1, xc, yc, true);
      generate_square(subroot, scale-1, x,  yc, true);
    } else {
      generate_square(root, scale-1, x,  y , false);
      generate_square(root, scale-1, xc, y , false);
      generate_square(root, scale-1, xc, yc, false);
      generate_square(root, scale-1, x,  yc, false);
    }
  } else if (subdivide(scale, x+size, y)) {
    add_quad(POINTG,POINTA,POINTI,POINTF);
    add_quad(POINTA,POINTA,POINTF,POINTC);
  } else if (subdivide(scale, x-size, y)) {
    add_quad(POINTG,POINTD,POINTI,POINTI);
    add_quad(POINTD,POINTA,POINTI,POINTC);
  } else if (subdivide(scale, x, y+size)) {
    add_quad(POINTG,POINTA,POINTH,POINTA);
    add_quad(POINTH,POINTA,POINTI,POINTC);
  } else if (subdivide(scale, x, y-size)) {
    add_quad(POINTG,POINTA,POINTI,POINTB);
    add_quad(POINTI,POINTB,POINTI,POINTC);
  } else {
    add_quad(POINTG,POINTA,POINTI,POINTC);
  }
  if (forceclose || (_next_index > _max_triangles)) {
    close_geom(root);
  }
}

/**
 * Calculates how many triangles are inside the given region.  The result is
 * stored in the _poly_totals array, in the center of the square.
 */
int HeightfieldTesselator::
count_triangles(int scale, int x, int y) {
  int size = 1<<scale;
  if (subdivide(scale, x, y)) {
    int xc = x + (size>>1);
    int yc = y + (size>>1);
    int n = 0;
    n += count_triangles(scale-1, x,  y );
    n += count_triangles(scale-1, xc, y );
    n += count_triangles(scale-1, x,  yc);
    n += count_triangles(scale-1, xc, yc);
    _triangle_totals[yc*_heightfield.get_x_size() + xc] = n;
    return n;
  } else if (subdivide(scale, x+size, y)) {
    return 3;
  } else if (subdivide(scale, x-size, y)) {
    return 3;
  } else if (subdivide(scale, x, y+size)) {
    return 3;
  } else if (subdivide(scale, x, y-size)) {
    return 3;
  } else {
    return 2;
  }
}

/**
 * Adds a quad to the current triangle strip.
 */
void HeightfieldTesselator::
add_quad_to_strip(int v1, int v2, int v3, int v4) {
  if ((v1 != v2)&&(v2 != v3)&&(v1 != v3)) {
    _triangles->add_vertices(v1,v3,v2);
  }
  if ((v3 != v2)&&(v2 != v4)&&(v4 != v3)) {
    _triangles->add_vertices(v3,v4,v2);
  }
}

/**
 * Adds a quad to the current geom.
 *
 * Eventually, I plan to reimplement this.  It is going to add a quad to a
 * table of quads.  A post-processing pass is going to traverse the table,
 * calling add_quad_to_strip in the optimal order.  For now, though, this
 * routine just calls add_quad_to_strip directly, which is quite inefficient.
 *
 */
void HeightfieldTesselator::
add_quad(int v1, int v2, int v3, int v4) {
  add_quad_to_strip(v1, v2, v3, v4);
}


/**
 * Initiates the construction of a geom.
 */
void HeightfieldTesselator::
open_geom() {
  _vdata = new GeomVertexData
    ("heightfield", GeomVertexFormat::get_v3n3(), Geom::UH_static);
  _vertex_writer = new GeomVertexWriter(_vdata, InternalName::get_vertex());
  _normal_writer = new GeomVertexWriter(_vdata, InternalName::get_normal());
  _triangles = new GeomTriangles(Geom::UH_static);
  _triangles->set_shade_model(Geom::SM_uniform);

  _next_index = 0;
  _last_vertex_a = -1;
  _last_vertex_b = -1;
}

/**
 * Completes the construction of a geom.
 */
void HeightfieldTesselator::
close_geom(NodePath root) {
  if (_triangles == nullptr) {
    return;
  }
  _triangles->close_primitive();
  PT(Geom) geom = new Geom(_vdata);
  geom->add_primitive(_triangles);
  PT(GeomNode) gnode = new GeomNode(get_name() + " patch");
  gnode->add_geom(geom);
  root.attach_new_node(gnode);
  delete _vertex_writer;
  delete _normal_writer;

  for (int i=0; i<_next_index; i++) {
    _vertex_index[_dirty_vertices[i]] = -1;
  }

  _next_index = 0;
  _last_vertex_a = -1;
  _last_vertex_b = -1;
  _vertex_writer = nullptr;
  _normal_writer = nullptr;
  _triangles = nullptr;
}
