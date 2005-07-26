// Filename: mayaEggLoader.h
// Created by:  jyelon (20jul05)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2005, Disney Enterprises, Inc.  All rights reserved
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

#ifndef MAYAEGGLOADER_H
#define MAYAEGGLOADER_H

class EggData;

bool MayaLoadEggData(EggData *data,    bool merge, bool model, bool anim);
bool MayaLoadEggFile(const char *name, bool merge, bool model, bool anim);

#endif

