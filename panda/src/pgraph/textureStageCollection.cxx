// Filename: textureStageCollection.cxx
// Created by:  drose (23Jul04)
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

#include "textureStageCollection.h"

#include "indent.h"
#include "indirectLess.h"
#include <algorithm>

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextureStageCollection::
TextureStageCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextureStageCollection::
TextureStageCollection(const TextureStageCollection &copy) :
  _texture_stages(copy._texture_stages)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
operator = (const TextureStageCollection &copy) {
  _texture_stages = copy._texture_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::add_texture_stage
//       Access: Published
//  Description: Adds a new TextureStage to the collection.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
add_texture_stage(TextureStage *node_texture_stage) {
  // If the pointer to our internal array is shared by any other
  // TextureStageCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren TextureStageCollection
  // objects.

  if (_texture_stages.get_ref_count() > 1) {
    TextureStages old_texture_stages = _texture_stages;
    _texture_stages = TextureStages::empty_array(0);
    _texture_stages.v() = old_texture_stages.v();
  }

  _texture_stages.push_back(node_texture_stage);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::remove_texture_stage
//       Access: Published
//  Description: Removes the indicated TextureStage from the collection.
//               Returns true if the texture_stage was removed, false if it was
//               not a member of the collection.
////////////////////////////////////////////////////////////////////
bool TextureStageCollection::
remove_texture_stage(TextureStage *node_texture_stage) {
  int texture_stage_index = -1;
  for (int i = 0; texture_stage_index == -1 && i < (int)_texture_stages.size(); i++) {
    if (_texture_stages[i] == node_texture_stage) {
      texture_stage_index = i;
    }
  }

  if (texture_stage_index == -1) {
    // The indicated texture_stage was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // TextureStageCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren TextureStageCollection
  // objects.

  if (_texture_stages.get_ref_count() > 1) {
    TextureStages old_texture_stages = _texture_stages;
    _texture_stages = TextureStages::empty_array(0);
    _texture_stages.v() = old_texture_stages.v();
  }

  _texture_stages.erase(_texture_stages.begin() + texture_stage_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::add_texture_stages_from
//       Access: Published
//  Description: Adds all the TextureStages indicated in the other
//               collection to this texture_stage.  The other texture_stages are simply
//               appended to the end of the texture_stages in this list;
//               duplicates are not automatically removed.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
add_texture_stages_from(const TextureStageCollection &other) {
  int other_num_texture_stages = other.get_num_texture_stages();
  for (int i = 0; i < other_num_texture_stages; i++) {
    add_texture_stage(other.get_texture_stage(i));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::remove_texture_stages_from
//       Access: Published
//  Description: Removes from this collection all of the TextureStages
//               listed in the other collection.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
remove_texture_stages_from(const TextureStageCollection &other) {
  TextureStages new_texture_stages;
  int num_texture_stages = get_num_texture_stages();
  for (int i = 0; i < num_texture_stages; i++) {
    PT(TextureStage) texture_stage = get_texture_stage(i);
    if (!other.has_texture_stage(texture_stage)) {
      new_texture_stages.push_back(texture_stage);
    }
  }
  _texture_stages = new_texture_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::remove_duplicate_texture_stages
//       Access: Published
//  Description: Removes any duplicate entries of the same TextureStages
//               on this collection.  If a TextureStage appears multiple
//               times, the first appearance is retained; subsequent
//               appearances are removed.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
remove_duplicate_texture_stages() {
  TextureStages new_texture_stages;

  int num_texture_stages = get_num_texture_stages();
  for (int i = 0; i < num_texture_stages; i++) {
    PT(TextureStage) texture_stage = get_texture_stage(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (texture_stage == get_texture_stage(j));
    }

    if (!duplicated) {
      new_texture_stages.push_back(texture_stage);
    }
  }

  _texture_stages = new_texture_stages;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::has_texture_stage
//       Access: Published
//  Description: Returns true if the indicated TextureStage appears in
//               this collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool TextureStageCollection::
has_texture_stage(TextureStage *texture_stage) const {
  for (int i = 0; i < get_num_texture_stages(); i++) {
    if (texture_stage == get_texture_stage(i)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::clear
//       Access: Published
//  Description: Removes all TextureStages from the collection.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
clear() {
  _texture_stages.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::find_texture_stage
//       Access: Published
//  Description: Returns the texture_stage in the collection with the
//               indicated name, if any, or NULL if no texture_stage has
//               that name.
////////////////////////////////////////////////////////////////////
TextureStage *TextureStageCollection::
find_texture_stage(const string &name) const {
  int num_texture_stages = get_num_texture_stages();
  for (int i = 0; i < num_texture_stages; i++) {
    TextureStage *texture_stage = get_texture_stage(i);
    if (texture_stage->get_name() == name) {
      return texture_stage;
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::get_num_texture_stages
//       Access: Published
//  Description: Returns the number of TextureStages in the collection.
////////////////////////////////////////////////////////////////////
int TextureStageCollection::
get_num_texture_stages() const {
  return _texture_stages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::get_texture_stage
//       Access: Published
//  Description: Returns the nth TextureStage in the collection.
////////////////////////////////////////////////////////////////////
TextureStage *TextureStageCollection::
get_texture_stage(int index) const {
  nassertr(index >= 0 && index < (int)_texture_stages.size(), NULL);

  return _texture_stages[index];
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::operator []
//       Access: Published
//  Description: Returns the nth TextureStage in the collection.  This is
//               the same as get_texture_stage(), but it may be a more
//               convenient way to access it.
////////////////////////////////////////////////////////////////////
TextureStage *TextureStageCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_texture_stages.size(), NULL);

  return _texture_stages[index];
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::size
//       Access: Published
//  Description: Returns the number of texture stages in the
//               collection.  This is the same thing as
//               get_num_texture_stages().
////////////////////////////////////////////////////////////////////
int TextureStageCollection::
size() const {
  return _texture_stages.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::sort
//       Access: Published
//  Description: Sorts the TextureStages in this collection into order
//               by TextureStage::sort(), from lowest to highest.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
sort() {
  ::sort(_texture_stages.begin(), _texture_stages.end(),
         CompareTextureStageSort());
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::output
//       Access: Published
//  Description: Writes a brief one-line description of the
//               TextureStageCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
output(ostream &out) const {
  if (get_num_texture_stages() == 1) {
    out << "1 TextureStage";
  } else {
    out << get_num_texture_stages() << " TextureStages";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStageCollection::write
//       Access: Published
//  Description: Writes a complete multi-line description of the
//               TextureStageCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void TextureStageCollection::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_texture_stages(); i++) {
    indent(out, indent_level) << *get_texture_stage(i) << "\n";
  }
}
