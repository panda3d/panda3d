// Filename: textureStageCollection.h
// Created by:  drose (23Jul04)
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

#ifndef TEXTURESTAGECOLLECTION_H
#define TEXTURESTAGECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "textureStage.h"

////////////////////////////////////////////////////////////////////
//       Class : TextureStageCollection
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextureStageCollection {
PUBLISHED:
  TextureStageCollection();
  TextureStageCollection(const TextureStageCollection &copy);
  void operator = (const TextureStageCollection &copy);
  INLINE ~TextureStageCollection();

  void add_texture_stage(TextureStage *node_texture_stage);
  bool remove_texture_stage(TextureStage *node_texture_stage);
  void add_texture_stages_from(const TextureStageCollection &other);
  void remove_texture_stages_from(const TextureStageCollection &other);
  void remove_duplicate_texture_stages();
  bool has_texture_stage(TextureStage *texture_stage) const;
  void clear();

  TextureStage *find_texture_stage(const string &name) const;

  int get_num_texture_stages() const;
  TextureStage *get_texture_stage(int index) const;
  TextureStage *operator [] (int index) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(TextureStage)) TextureStages;
  TextureStages _texture_stages;
};

INLINE ostream &operator << (ostream &out, const TextureStageCollection &col) {
  col.output(out);
  return out;
}

#include "textureStageCollection.I"

#endif


