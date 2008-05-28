// Filename: mayaEggLoader.h
// Created by:  jyelon (20jul05)
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

#ifndef MAYAEGGLOADER_H
#define MAYAEGGLOADER_H

class EggData;

bool MayaLoadEggData(EggData *data,    bool merge, bool model, bool anim, bool respect_normals);
bool MayaLoadEggFile(const char *name, bool merge, bool model, bool anim, bool respect_normals);

#endif

