// Filename: eggTrans.h
// Created by:  drose (14Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef EGGTRANS_H
#define EGGTRANS_H

#include <pandatoolbase.h>

#include <eggFilter.h>

////////////////////////////////////////////////////////////////////
//       Class : EggTrans
// Description : A program to read an egg file and write an equivalent
//               egg file, possibly performing some minor operations
//               along the way.
////////////////////////////////////////////////////////////////////
class EggTrans : public EggFilter {
public:
  EggTrans();

  void run();

  bool _flatten_transforms;
  bool _apply_texmats;
  bool _collapse_equivalent_textures;
  bool _remove_invalid_primitives;
  bool _triangulate_polygons;
  bool _standardize_names;
};

#endif

