// Filename: triangleMesh.h
// Created by:  drose (06Nov99)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include <luse.h>
#include <pta_Vertexf.h>
#include <pta_Normalf.h>
#include <pta_Colorf.h>
#include <pta_TexCoordf.h>

class GeomTristrip;

class TriangleMesh {
public:
  TriangleMesh(int x_verts, int y_verts);

  int get_x_verts() const;
  int get_y_verts() const;
  int get_num_verts() const;

  GeomTristrip *build_mesh() const;

  PTA_Vertexf _coords;
  PTA_Normalf _norms;
  PTA_Colorf _colors;
  PTA_TexCoordf _texcoords;

private:
  int _x_verts, _y_verts;
};

#endif

  
