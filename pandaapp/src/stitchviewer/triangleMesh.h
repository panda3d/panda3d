// Filename: triangleMesh.h
// Created by:  drose (06Nov99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef TRIANGLEMESH_H
#define TRIANGLEMESH_H

#include "luse.h"
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


