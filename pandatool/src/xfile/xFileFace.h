// Filename: xFileFace.h
// Created by:  drose (19Jun01)
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

#ifndef XFILEFACE_H
#define XFILEFACE_H

#include "pandatoolbase.h"
#include "pvector.h"

class XFileMesh;
class EggPolygon;

////////////////////////////////////////////////////////////////////
//       Class : XFileFace
// Description : This represents a single face of an XFileMesh.
////////////////////////////////////////////////////////////////////
class XFileFace {
public:
  XFileFace();
  void set_from_egg(XFileMesh *mesh, EggPolygon *egg_poly);

  class Vertex {
  public:
    int _vertex_index;
    int _normal_index;
  };
  typedef pvector<Vertex> Vertices;
  Vertices _vertices;

  int _material_index;
};

#endif

