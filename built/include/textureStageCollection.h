/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureStageCollection.h
 * @author drose
 * @date 2004-07-23
 */

#ifndef TEXTURESTAGECOLLECTION_H
#define TEXTURESTAGECOLLECTION_H

#include "pandabase.h"
#include "pointerToArray.h"
#include "textureStage.h"

/**
 *
 */
class EXPCL_PANDA_PGRAPH TextureStageCollection {
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

  TextureStage *find_texture_stage(const std::string &name) const;

  int get_num_texture_stages() const;
  TextureStage *get_texture_stage(int index) const;
  MAKE_SEQ(get_texture_stages, get_num_texture_stages, get_texture_stage);
  TextureStage *operator [] (int index) const;
  int size() const;
  INLINE void operator += (const TextureStageCollection &other);
  INLINE TextureStageCollection operator + (const TextureStageCollection &other) const;

  void sort();

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level = 0) const;

private:
  typedef PTA(PT(TextureStage)) TextureStages;
  TextureStages _texture_stages;

  class CompareTextureStageSort {
  public:
    INLINE bool operator () (const TextureStage *a, const TextureStage *b) const;
  };

};

INLINE std::ostream &operator << (std::ostream &out, const TextureStageCollection &col) {
  col.output(out);
  return out;
}

#include "textureStageCollection.I"

#endif
