/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggBinner.h
 * @author drose
 * @date 2000-02-17
 */

#ifndef EGGBINNER_H
#define EGGBINNER_H

#include "pandabase.h"

#include "eggBinMaker.h"

class EggLoader;

/**
 * A special binner used only within this package to pre-process the egg tree
 * for the loader and group things together as appropriate.
 *
 * It is used to collect similar polygons together for a Geom, as well as to
 * group related LOD children together under a single LOD node.
 */
class EXPCL_PANDA_EGG2PG EggBinner : public EggBinMaker {
public:
  // The BinNumber serves to identify why a particular EggBin was created.
  enum BinNumber {
    BN_none = 0,
    BN_polyset,
    BN_lod,
    BN_nurbs_surface,
    BN_nurbs_curve,
    BN_patches,
  };

  EggBinner(EggLoader &loader);

  virtual void
  prepare_node(EggNode *node);

  virtual int
  get_bin_number(const EggNode *node);

  virtual std::string
  get_bin_name(int bin_number, const EggNode *child);

  virtual bool
  sorts_less(int bin_number, const EggNode *a, const EggNode *b);

  EggLoader &_loader;
};


#endif
