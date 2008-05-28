// Filename: dxfToEggConverter.cxx
// Created by:  drose (04May04)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "dxfToEggConverter.h"
#include "dxfToEggLayer.h"
#include "eggData.h"

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXFToEggConverter::
DXFToEggConverter() {
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXFToEggConverter::
DXFToEggConverter(const DXFToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
DXFToEggConverter::
~DXFToEggConverter() {
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *DXFToEggConverter::
make_copy() {
  return new DXFToEggConverter(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string DXFToEggConverter::
get_name() const {
  return "DXF";
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string DXFToEggConverter::
get_extension() const {
  return "dxf";
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DXFToEggConverter::
supports_compressed() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool DXFToEggConverter::
convert_file(const Filename &filename) {
  clear_error();

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_zup_right);
  }

  process(filename);
  return !had_error();
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::new_layer
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
DXFLayer *DXFToEggConverter::
new_layer(const string &name) {
  return new DXFToEggLayer(name, get_egg_data());
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::done_entity
//       Access: Protected, Virtual
//  Description: If the entity is a polygon, creates the corresponding
//               egg polygon.
////////////////////////////////////////////////////////////////////
void DXFToEggConverter::
done_entity() {
  if (_entity == EN_polyline) {
    // A Polyline is either an unclosed series of connected line
    // segments, or a closed polygon of arbitrary complexity.

    if ((_flags & PF_3d) == 0) {
      // it's a 2-d polygon; convert it to 3-d coordinates.
      ocs_2_wcs();
    }

    if (_flags & PF_closed) {
      // it's closed; create a polygon.
      nassertv(_layer!=NULL);
      ((DXFToEggLayer *)_layer)->add_polygon(this);
    } else {
      // It's open; create a series of line segments.
      nassertv(_layer!=NULL);
      ((DXFToEggLayer *)_layer)->add_line(this);
    }

  } else if (_entity == EN_3dface) {
    // DXF can also represent a polygon as a 3DFace.  This might be
    // either a quad or a triangle (if two of the vertices are the
    // same).  We'll add the vertices to our list of vertices and then
    // define the polygon.
    _verts.clear();
    _verts.push_back(DXFVertex(_s));
    _verts.push_back(DXFVertex(_r));
    _verts.push_back(DXFVertex(_q));
    _verts.push_back(DXFVertex(_p));
    
    nassertv(_layer!=NULL);
    ((DXFToEggLayer *)_layer)->add_polygon(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DXFToEggConverter::done_entity
//       Access: Protected, Virtual
//  Description: A hook for user code, if desired.  This function is
//               called when some unexpected error occurs while
//               reading the DXF file.
////////////////////////////////////////////////////////////////////
void DXFToEggConverter::
error() {
  _error = true;
}
