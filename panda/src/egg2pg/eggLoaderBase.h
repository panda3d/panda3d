// Filename: eggLoaderBase.h
// Created by:  drose (06Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef EGGLOADERBASE_H
#define EGGLOADERBASE_H

#include "pandabase.h"
#include "luse.h"

class EggPrimitive;
class PandaNode;
class NamedNode;
class ComputedVerticesMaker;

///////////////////////////////////////////////////////////////////
//       Class : EggLoaderBase
// Description : QP: A temporary hack around having to have two kinds
//               of EggLoaders and one kind of CharacterMaker, this
//               presents the interface to both kinds of EggLoaders.
////////////////////////////////////////////////////////////////////
class EggLoaderBase {
public:
  virtual ~EggLoaderBase() { }

  virtual void make_nonindexed_primitive(EggPrimitive *egg_prim, PandaNode *parent,
                                         const LMatrix4d *transform = NULL) { }

  virtual void make_indexed_primitive(EggPrimitive *egg_prim, PandaNode *parent,
                                      const LMatrix4d *transform,
                                      ComputedVerticesMaker &_comp_verts_maker) { }

  virtual void make_nonindexed_primitive(EggPrimitive *egg_prim, NamedNode *parent,
                                         const LMatrix4d *transform = NULL) { }

  virtual void make_indexed_primitive(EggPrimitive *egg_prim, NamedNode *parent,
                                      const LMatrix4d *transform,
                                      ComputedVerticesMaker &_comp_verts_maker) { }
};

#endif

