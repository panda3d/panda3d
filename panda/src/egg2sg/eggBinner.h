// Filename: eggBinner.h
// Created by:  drose (17Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef EGGBINNER_H
#define EGGBINNER_H

#include <pandabase.h>

#include <eggBinMaker.h>

///////////////////////////////////////////////////////////////////
//       Class : EggBinner
// Description : A special binner used only within this package to
//               pre-process the egg tree for the loader and group
//               things together as appropriate.
//
//               Presently, it only groups related LOD children
//               together under a single LOD node.
////////////////////////////////////////////////////////////////////
class EggBinner : public EggBinMaker {
public:

  // The BinNumber serves to identify why a particular EggBin was
  // created.
  enum BinNumber {
    BN_none = 0,
    BN_lod,
  };

  virtual int
  get_bin_number(const EggNode *node);

  virtual bool
  sorts_less(int bin_number, const EggNode *a, const EggNode *b);

  virtual bool
  collapse_group(const EggGroup *group, int bin_number);
};


#endif
