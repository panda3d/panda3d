/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureCollection.cxx
 * @author drose
 * @date 2002-03-16
 */

#include "textureCollection.h"
#include "indent.h"

/**
 *
 */
TextureCollection::
TextureCollection() {
}

/**
 *
 */
TextureCollection::
TextureCollection(const TextureCollection &copy) :
  _textures(copy._textures)
{
}

/**
 *
 */
void TextureCollection::
operator = (const TextureCollection &copy) {
  _textures = copy._textures;
}

/**
 * Adds a new Texture to the collection.
 */
void TextureCollection::
add_texture(Texture *texture) {
  // If the pointer to our internal array is shared by any other
  // TextureCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren TextureCollection objects.

  if (_textures.get_ref_count() > 1) {
    Textures old_textures = _textures;
    _textures = Textures::empty_array(0);
    _textures.v() = old_textures.v();
  }

  _textures.push_back(texture);
}

/**
 * Removes the indicated Texture from the collection.  Returns true if the
 * texture was removed, false if it was not a member of the collection.
 */
bool TextureCollection::
remove_texture(Texture *texture) {
  int texture_index = -1;
  for (int i = 0; texture_index == -1 && i < (int)_textures.size(); i++) {
    if (_textures[i] == texture) {
      texture_index = i;
    }
  }

  if (texture_index == -1) {
    // The indicated texture was not a member of the collection.
    return false;
  }

  // If the pointer to our internal array is shared by any other
  // TextureCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren TextureCollection objects.

  if (_textures.get_ref_count() > 1) {
    Textures old_textures = _textures;
    _textures = Textures::empty_array(0);
    _textures.v() = old_textures.v();
  }

  _textures.erase(_textures.begin() + texture_index);
  return true;
}

/**
 * Adds all the Textures indicated in the other collection to this texture.
 * The other textures are simply appended to the end of the textures in this
 * list; duplicates are not automatically removed.
 */
void TextureCollection::
add_textures_from(const TextureCollection &other) {
  int other_num_textures = other.get_num_textures();
  for (int i = 0; i < other_num_textures; i++) {
    add_texture(other.get_texture(i));
  }
}


/**
 * Removes from this collection all of the Textures listed in the other
 * collection.
 */
void TextureCollection::
remove_textures_from(const TextureCollection &other) {
  Textures new_textures;
  int num_textures = get_num_textures();
  for (int i = 0; i < num_textures; i++) {
    PT(Texture) texture = get_texture(i);
    if (!other.has_texture(texture)) {
      new_textures.push_back(texture);
    }
  }
  _textures = new_textures;
}

/**
 * Removes any duplicate entries of the same Textures on this collection.  If
 * a Texture appears multiple times, the first appearance is retained;
 * subsequent appearances are removed.
 */
void TextureCollection::
remove_duplicate_textures() {
  Textures new_textures;

  int num_textures = get_num_textures();
  for (int i = 0; i < num_textures; i++) {
    PT(Texture) texture = get_texture(i);
    bool duplicated = false;

    for (int j = 0; j < i && !duplicated; j++) {
      duplicated = (texture == get_texture(j));
    }

    if (!duplicated) {
      new_textures.push_back(texture);
    }
  }

  _textures = new_textures;
}

/**
 * Returns true if the indicated Texture appears in this collection, false
 * otherwise.
 */
bool TextureCollection::
has_texture(Texture *texture) const {
  for (int i = 0; i < get_num_textures(); i++) {
    if (texture == get_texture(i)) {
      return true;
    }
  }
  return false;
}

/**
 * Removes all Textures from the collection.
 */
void TextureCollection::
clear() {
  _textures.clear();
}

/**
 * This is a hint to Panda to allocate enough memory to hold the given number
 * of NodePaths, if you know ahead of time how many you will be adding.
 */
void TextureCollection::
reserve(size_t num) {
  _textures.reserve(num);
}

/**
 * Returns the texture in the collection with the indicated name, if any, or
 * NULL if no texture has that name.
 */
Texture *TextureCollection::
find_texture(const std::string &name) const {
  int num_textures = get_num_textures();
  for (int i = 0; i < num_textures; i++) {
    Texture *texture = get_texture(i);
    if (texture->get_name() == name) {
      return texture;
    }
  }
  return nullptr;
}

/**
 * Returns the number of Textures in the collection.
 */
int TextureCollection::
get_num_textures() const {
  return _textures.size();
}

/**
 * Returns the nth Texture in the collection.
 */
Texture *TextureCollection::
get_texture(int index) const {
  nassertr(index >= 0 && index < (int)_textures.size(), nullptr);

  return _textures[index];
}

/**
 * Returns the nth Texture in the collection.  This is the same as
 * get_texture(), but it may be a more convenient way to access it.
 */
Texture *TextureCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_textures.size(), nullptr);

  return _textures[index];
}

/**
 * Returns the number of textures in the collection.  This is the same thing
 * as get_num_textures().
 */
int TextureCollection::
size() const {
  return _textures.size();
}

/**
 * Writes a brief one-line description of the TextureCollection to the
 * indicated output stream.
 */
void TextureCollection::
output(std::ostream &out) const {
  if (get_num_textures() == 1) {
    out << "1 Texture";
  } else {
    out << get_num_textures() << " Textures";
  }
}

/**
 * Writes a complete multi-line description of the TextureCollection to the
 * indicated output stream.
 */
void TextureCollection::
write(std::ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_textures(); i++) {
    indent(out, indent_level) << *get_texture(i) << "\n";
  }
}
