// Filename: triangleMesh.h
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include <luse.h>
#include <pointerToArray.h>

class GeomTristrip;

class TriangleMesh {
public:
  TriangleMesh(int x_verts, int y_verts);

  int get_x_verts() const;
  int get_y_verts() const;
  int get_num_verts() const;

  GeomTristrip *build_mesh() const;

  PTA(Vertexf) _coords;
  PTA(Normalf) _norms;
  PTA(Colorf) _colors;
  PTA(TexCoordf) _texcoords;

private:
  int _x_verts, _y_verts;
};

#endif

  
