// Filename: rotate_to.h
// Created by:  drose (04Nov99)
//
////////////////////////////////////////////////////////////////////

#ifndef ROTATE_TO_H
#define ROTATE_TO_H

////////////////////////////////////////////////////////////////////
//
// rotate_to()
//
// This function computes a suitable rotation matrix to rotate vector
// a onto vector b.  That is, it computes mat so that a * mat = b.
// The rotation axis is chosen to give the smallest possible rotation
// angle.
//
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "lmatrix.h"
#include "luse.h"

void rotate_to(LMatrix3f &mat, const LVector3f &a, const LVector3f &b);
void rotate_to(LMatrix3d &mat, const LVector3d &a, const LVector3d &b);

void rotate_to(LMatrix4f &mat, const LVector3f &a, const LVector3f &b);
void rotate_to(LMatrix4d &mat, const LVector3d &a, const LVector3d &b);

#endif
