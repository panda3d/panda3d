// Filename: eggBinner.h
// Created by:  drose (17Feb00)
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

#ifndef EGGBINNER_H
#define EGGBINNER_H

#include "pandabase.h"

#include "eggBinMaker.h"

class EggLoader;

////////////////////////////////////////////////////////////////////
//       Class : EggBinner
// Description : A special binner used only within this package to
//               pre-process the egg tree for the loader and group
//               things together as appropriate.
//
//               It is used to collect similar polygons together for a
//               Geom, as well as to group related LOD children
//               together under a single LOD node.
////////////////////////////////////////////////////////////////////
class EggBinner : public EggBinMaker {
public:
  // The BinNumber serves to identify why a particular EggBin was
  // created.
  enum BinNumber {
    BN_none = 0,
    BN_polyset,
    BN_lod,
    BN_nurbs_surface,
    BN_nurbs_curve,
  };

  EggBinner(EggLoader &loader);

  virtual void
  prepare_node(EggNode *node);

  virtual int
  get_bin_number(const EggNode *node);

  virtual string
  get_bin_name(int bin_number, const EggNode *child);

  virtual bool
  sorts_less(int bin_number, const EggNode *a, const EggNode *b);

  EggLoader &_loader;
};


#endif
