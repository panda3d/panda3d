// Filename: dxfLayer.h
// Created by:  drose (04May04)
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

#ifndef DXFLAYER_H
#define DXFLAYER_H

#include "pandatoolbase.h"
#include "dxfVertexMap.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DXFLayer
// Description : This represents a "layer" as read from the DXF file.
//               A layer may be defined by reading the header part of
//               the file, or it may be implicitly defined by an
//               entity's having referenced it.
//
//               The DXFLayer class provides a facility for collapsing
//               identical vertices, via the _vmap member; however,
//               this is not automatic and must be done explicitly by
//               user code.
//
//               User code may derive from DXFLayer to associate
//               private data with each layer, if desired.
////////////////////////////////////////////////////////////////////
class DXFLayer {
public:
  DXFLayer(const string &name) : _name(name) { }
  virtual ~DXFLayer() { }
  
  string _name;
  DXFVertexMap _vmap;
};

#endif
