// Filename: physxHeightFieldDesc.cxx
// Created by:  enn0x (15Oct09)
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

#include "physxHeightFieldDesc.h"

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_convex_edge_threshold
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldDesc::
set_convex_edge_threshold(float threshold) {

  _desc.convexEdgeThreshold = threshold;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_thickness
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldDesc::
set_thickness(float thickness) {

  _desc.thickness = thickness;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_image
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldDesc::
set_image(const PNMImage &image, unsigned short materialIndex) {

  NxU32 _32K = 32767; // (1<<15)-1;
  NxU32 _64K = 65535; // (1<<16)-1;

  NxU32 nbRows = image.get_x_size();
  NxU32 nbColumns = image.get_y_size();

  set_size(nbRows, nbColumns);

  NxU8 *currentByte = (NxU8 *)(_desc.samples);

  for (NxU32 row=0; row < nbRows; row++) {
    for (NxU32 column=0; column < nbColumns; column++) {

      NxHeightFieldSample* currentSample = (NxHeightFieldSample *)currentByte;

      NxReal fvalue = image.get_bright(row, column);
      NxI16 ivalue = (NxI16)((fvalue - 0.5f) * _32K);

      currentSample->height         = (NxI16)ivalue;
      currentSample->tessFlag       = (NxU8)0;
      currentSample->materialIndex0 = (NxU8)materialIndex;
      currentSample->materialIndex1 = (NxU8)materialIndex;

      currentByte += _desc.sampleStride;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_material_index
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldDesc::
set_material_index(unsigned int row, unsigned int column, unsigned short materialIndex0, unsigned short materialIndex1) {

  nassertv(_desc.samples);
  nassertv(row < _desc.nbRows);
  nassertv(column < _desc.nbColumns);

  NxU32 idx = row * _desc.nbColumns + column;
  NxU8 *byte = (NxU8 *)_desc.samples + _desc.sampleStride * idx;
  NxHeightFieldSample* sample = (NxHeightFieldSample *)byte;

  //NxHeightFieldSample* sample = ((NxHeightFieldSample *)_desc.samples) + idx;

  sample->materialIndex0 = (NxU8)materialIndex0;
  sample->materialIndex1 = (NxU8)materialIndex1;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_height
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldDesc::
set_height(unsigned int row, unsigned int column, short height) {

  nassertv(_desc.samples);
  nassertv(row < _desc.nbRows);
  nassertv(column < _desc.nbColumns);

  NxU32 idx = row * _desc.nbColumns + column;
  NxU8 *byte = (NxU8 *)_desc.samples + _desc.sampleStride * idx;
  NxHeightFieldSample* sample = (NxHeightFieldSample *)byte;

  sample->height = (NxI16)height;
}

////////////////////////////////////////////////////////////////////
//     Function: PhysxHeightFieldDesc::set_tess_flag
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void PhysxHeightFieldDesc::
set_tess_flag(unsigned int row, unsigned int column, unsigned short value) {

  nassertv(_desc.samples);
  nassertv(row < _desc.nbRows);
  nassertv(column < _desc.nbColumns);

  NxU32 idx = row * _desc.nbColumns + column;
  NxU8 *byte = (NxU8 *)_desc.samples + _desc.sampleStride * idx;
  NxHeightFieldSample* sample = (NxHeightFieldSample *)byte;

  nassertv(value < 2);
  sample->tessFlag = (NxU8)value;
}

