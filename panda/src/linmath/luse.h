// Filename: luse.h
// Created by:  drose (13Jan99)
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

#ifndef LUSE_H
#define LUSE_H

////////////////////////////////////////////////////////////////////
//
// This file defines a number of vector-based classes that are
// designed for specific uses.  These all inherit from
// LVecBase[234][fd], which is the base of all linear algebra vectors.
//
// LPoint[234][fd]
//
//   This should be used to represent a specific point in space.  It
//   inherits most properties from LVecBase.
//
// LVector[234][fd]
//
//   This should be used to represent a vector, or a distance between
//   two points in space.
//
// The distinction between LPoint and LVector is worth emphasizing.
// They differ in some subtle typing behavior (vector - vector =
// vector, point + vector = point, point - point = vector) and also in
// the way they are transformed when multiplied by a matrix (a point
// gets the translation component of the matrix, while the vector does
// not).  Also, vector has length() and normalize() functions defined
// for it, while point does not.
//
// LPoint and LVector should be used whenever the concept of "point"
// or "vector" applies.  If neither applies--for instance, if you are
// storing a plane equation or some such nonsense--use the base class,
// LVecBase.
//
// This file also typedefs the following:
//
// Vertex[fd]
// Normal[fd]
// TexCoord[fd]
// Color[fd]
// RGBColor[fd]
//
// These classes are typedefs of LPoint or LVector, as appropriate,
// and are intended to store a specific kind of rendering attribute.
// (Color is a four-component color; RGBColor is three-component.)
//
////////////////////////////////////////////////////////////////////

// All of the guts is actually defined in this other header file,
// which is not intended to be included directly by the user.
#include "aa_luse.h"

#endif

