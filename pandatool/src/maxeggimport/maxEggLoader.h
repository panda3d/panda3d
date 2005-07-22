// Filename: maxEggLoader.h
// Created by:  jyelon (15jul05)
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

#ifndef MAXEGGLOADER_H
#define MAXEGGLOADER_H

class EggData;

bool MaxLoadEggData(EggData *data,    bool merge, bool model, bool anim);
bool MaxLoadEggFile(const char *name, bool merge, bool model, bool anim);

#endif

