// Filename: maxEggLoader.h
// Created by:  jyelon (15jul05)
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

#ifndef MAXEGGLOADER_H
#define MAXEGGLOADER_H

class EggData;

bool MaxLoadEggData(EggData *data,    bool merge, bool model, bool anim);
bool MaxLoadEggFile(const char *name, bool merge, bool model, bool anim);

#endif

