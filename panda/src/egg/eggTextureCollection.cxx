/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file eggTextureCollection.cxx
 * @author drose
 * @date 2000-02-15
 */

#include "eggTextureCollection.h"
#include "eggGroupNode.h"
#include "eggPrimitive.h"
#include "eggTexture.h"
#include "pt_EggTexture.h"
#include "dcast.h"

#include "nameUniquifier.h"

#include <algorithm>

/**
 *
 */
EggTextureCollection::
EggTextureCollection() {
}

/**
 *
 */
EggTextureCollection::
EggTextureCollection(const EggTextureCollection &copy) :
  _textures(copy._textures),
  _ordered_textures(copy._ordered_textures)
{
}

/**
 *
 */
EggTextureCollection &EggTextureCollection::
operator = (const EggTextureCollection &copy) {
  _textures = copy._textures;
  _ordered_textures = copy._ordered_textures;
  return *this;
}

/**
 *
 */
EggTextureCollection::
~EggTextureCollection() {
}

/**
 * Removes all textures from the collection.
 */
void EggTextureCollection::
clear() {
  _textures.clear();
  _ordered_textures.clear();
}

/**
 * Walks the egg hierarchy beginning at the indicated node, and removes any
 * EggTextures encountered in the hierarchy, adding them to the collection.
 * Returns the number of EggTextures encountered.
 */
int EggTextureCollection::
extract_textures(EggGroupNode *node) {
  // Since this traversal is destructive, we'll handle it within the
  // EggGroupNode code.
  return node->find_textures(this);
}


/**
 * Returns true if there are no EggTexures in the collection, false otherwise.
 */
bool EggTextureCollection::
is_empty() const {
  return _ordered_textures.empty();
}

/**
 * Returns the number of EggTextures in the collection.
 */
int EggTextureCollection::
get_num_textures() const {
  return _ordered_textures.size();
}

/**
 * Returns the nth EggTexture in the collection.
 */
EggTexture *EggTextureCollection::
get_texture(int index) const {
  nassertr(index >= 0 && index < (int)_ordered_textures.size(), nullptr);

  return _ordered_textures[index];
}

/**
 * Adds a series of EggTexture nodes to the beginning of the indicated node to
 * reflect each of the textures in the collection.  Returns an iterator
 * representing the first position after the newly inserted textures.
 */
EggGroupNode::iterator EggTextureCollection::
insert_textures(EggGroupNode *node) {
  return insert_textures(node, node->begin());
}

/**
 * Adds a series of EggTexture nodes to the beginning of the indicated node to
 * reflect each of the textures in the collection.  Returns an iterator
 * representing the first position after the newly inserted textures.
 */
EggGroupNode::iterator EggTextureCollection::
insert_textures(EggGroupNode *node, EggGroupNode::iterator position) {
  OrderedTextures::iterator oti;
  for (oti = _ordered_textures.begin();
       oti != _ordered_textures.end();
       ++oti) {
    EggTexture *texture = (*oti);
    position = node->insert(position, texture);
  }

  return position;
}

/**
 * Walks the egg hierarchy beginning at the indicated node, looking for
 * textures that are referenced by primitives but are not already members of
 * the collection, adding them to the collection.
 *
 * If this is called following extract_textures(), it can be used to pick up
 * any additional texture references that appeared in the egg hierarchy (but
 * whose EggTexture node was not actually part of the hierarchy).
 *
 * If this is called in lieu of extract_textures(), it will fill up the
 * collection with all of the referenced textures (and only the referenced
 * textures), without destructively removing the EggTextures from the
 * hierarchy.
 *
 * This also has the side effect of incrementing the internal usage count for
 * a texture in the collection each time a texture reference is encountered.
 * This side effect is taken advantage of by remove_unused_textures().
 *
 * And one more side effect: this function identifies the presence of
 * multitexturing in the egg file, and calls multitexture_over() on each
 * texture appropriately so that, after this call, you may expect
 * get_multitexture_sort() to return a reasonable value for each texture.
 */
int EggTextureCollection::
find_used_textures(EggNode *node) {
  int num_found = 0;

  if (node->is_of_type(EggPrimitive::get_class_type())) {
    EggPrimitive *primitive = DCAST(EggPrimitive, node);

    int num_textures = primitive->get_num_textures();
    for (int i = 0; i < num_textures; i++) {
      EggTexture *tex = primitive->get_texture(i);

      Textures::iterator ti = _textures.find(tex);
      if (ti == _textures.end()) {
        // Here's a new texture!
        num_found++;
        _textures.insert(Textures::value_type(tex, 1));
        _ordered_textures.push_back(tex);
      } else {
        // Here's a texture we'd already known about.  Increment its usage
        // count.
        (*ti).second++;
      }

      // Get the multitexture ordering right.
      for (int j = 0; j < i; j++) {
/*
 * The return value of this function will be false if there is some cycle in
 * the texture layout order; e.g.  A layers over B on one primitive, but B
 * layers over A on another primitive.  In that case the Egg Loader won't be
 * able to assign a unique ordering between A and B, so it's probably an error
 * worth reporting to the user--but we don't report it here, because this is a
 * much lower-level function that gets called in other contexts too.  That
 * means it doesn't get reported at all, but too bad.
 */
        tex->multitexture_over(primitive->get_texture(j));
      }
    }

  } else if (node->is_of_type(EggGroupNode::get_class_type())) {
    EggGroupNode *group = DCAST(EggGroupNode, node);

    EggGroupNode::iterator ci;
    for (ci = group->begin(); ci != group->end(); ++ci) {
      EggNode *child = *ci;

      num_found += find_used_textures(child);
    }
  }

  return num_found;
}

/**
 * Removes any textures from the collection that aren't referenced by any
 * primitives in the indicated egg hierarchy.  This also, incidentally, adds
 * textures to the collection that had been referenced by primitives but had
 * not previously appeared in the collection.
 */
void EggTextureCollection::
remove_unused_textures(EggNode *node) {
  // We'll do this the easy way: First, we'll remove *all* the textures from
  // the collection, and then we'll add back only those that appear in the
  // hierarchy.
  clear();
  find_used_textures(node);
}

/**
 * Walks through the collection and collapses together any separate textures
 * that are equivalent according to the indicated equivalence factor, eq (see
 * EggTexture::is_equivalent_to()).  The return value is the number of
 * textures removed.
 *
 * This flavor of collapse_equivalent_textures() automatically adjusts all the
 * primitives in the egg hierarchy to refer to the new texture pointers.
 */
int EggTextureCollection::
collapse_equivalent_textures(int eq, EggGroupNode *node) {
  TextureReplacement removed;
  int num_collapsed = collapse_equivalent_textures(eq, removed);

  // And now walk the egg hierarchy and replace any references to a removed
  // texture with its replacement.
  replace_textures(node, removed);

  return num_collapsed;
}

/**
 * Walks through the collection and collapses together any separate textures
 * that are equivalent according to the indicated equivalence factor, eq (see
 * EggTexture::is_equivalent_to()).  The return value is the number of
 * textures removed.
 *
 * This flavor of collapse_equivalent_textures() does not adjust any
 * primitives in the egg hierarchy; instead, it fills up the 'removed' map
 * with an entry for each removed texture, mapping it back to the equivalent
 * retained texture.  It's up to the user to then call replace_textures() with
 * this map, if desired, to apply these changes to the egg hierarchy.
 */
int EggTextureCollection::
collapse_equivalent_textures(int eq, EggTextureCollection::TextureReplacement &removed) {
  int num_collapsed = 0;

  typedef pset<PT_EggTexture, UniqueEggTextures> Collapser;
  UniqueEggTextures uet(eq);
  Collapser collapser(uet);

  // First, put all of the textures into the Collapser structure, to find out
  // the unique textures.
  OrderedTextures::const_iterator oti;
  for (oti = _ordered_textures.begin();
       oti != _ordered_textures.end();
       ++oti) {
    EggTexture *tex = (*oti);

    std::pair<Collapser::const_iterator, bool> result = collapser.insert(tex);
    if (!result.second) {
      // This texture is non-unique; another one was already there.
      EggTexture *first = *(result.first);
      removed.insert(TextureReplacement::value_type(tex, first));
      num_collapsed++;
    }
  }

  // Now record all of the unique textures only.
  clear();
  Collapser::const_iterator ci;
  for (ci = collapser.begin(); ci != collapser.end(); ++ci) {
    add_texture(*ci);
  }

  return num_collapsed;
}

/**
 * Walks the egg hierarchy, changing out any reference to a texture appearing
 * on the left side of the map with its corresponding texture on the right
 * side.  This is most often done following a call to
 * collapse_equivalent_textures().  It does not directly affect the
 * Collection.
 */
void EggTextureCollection::
replace_textures(EggGroupNode *node,
                 const EggTextureCollection::TextureReplacement &replace) {
  EggGroupNode::iterator ci;
  for (ci = node->begin();
       ci != node->end();
       ++ci) {
    EggNode *child = *ci;
    if (child->is_of_type(EggPrimitive::get_class_type())) {
      EggPrimitive *primitive = DCAST(EggPrimitive, child);
      EggPrimitive::Textures new_textures;
      EggPrimitive::Textures::const_iterator ti;
      for (ti = primitive->_textures.begin();
           ti != primitive->_textures.end();
           ++ti) {
        PT_EggTexture tex = (*ti);
        TextureReplacement::const_iterator ri;
        ri = replace.find(tex);
        if (ri != replace.end()) {
          // Here's a texture we want to replace.
          new_textures.push_back((*ri).second);
        } else {
          new_textures.push_back(tex);
        }
      }
      primitive->_textures.swap(new_textures);

    } else if (child->is_of_type(EggGroupNode::get_class_type())) {
      EggGroupNode *group_child = DCAST(EggGroupNode, child);
      replace_textures(group_child, replace);
    }
  }
}

/**
 * Guarantees that each texture in the collection has a unique TRef name.
 * This is essential before writing an egg file.
 */
void EggTextureCollection::
uniquify_trefs() {
  NameUniquifier nu(".tref", "tref");

  OrderedTextures::const_iterator oti;
  for (oti = _ordered_textures.begin();
       oti != _ordered_textures.end();
       ++oti) {
    EggTexture *tex = (*oti);

    tex->set_name(nu.add_name(tex->get_name()));
  }
}

/**
 * Sorts all the textures into alphabetical order by TRef name.  Subsequent
 * operations using begin()/end() will traverse in this sorted order.
 */
void EggTextureCollection::
sort_by_tref() {
  sort(_ordered_textures.begin(), _ordered_textures.end(),
       NamableOrderByName());
}

/**
 * Sorts all the textures into alphabetical order by the basename part
 * (including extension) of the filename.  Subsequent operations using
 * begin()/end() will traverse in this sorted order.
 */
void EggTextureCollection::
sort_by_basename() {
  sort(_ordered_textures.begin(), _ordered_textures.end(),
       EggFilenameNode::IndirectOrderByBasename());
}

/**
 * Explicitly adds a new texture to the collection.  Returns true if the
 * texture was added, false if it was already there or if there was some
 * error.
 */
bool EggTextureCollection::
add_texture(EggTexture *texture) {
  nassertr(_textures.size() == _ordered_textures.size(), false);

  PT_EggTexture new_tex = texture;

  Textures::const_iterator ti;
  ti = _textures.find(new_tex);
  if (ti != _textures.end()) {
    // This texture is already a member of the collection.
    return false;
  }

  _textures.insert(Textures::value_type(new_tex, 0));
  _ordered_textures.push_back(new_tex);

  nassertr(_textures.size() == _ordered_textures.size(), false);
  return true;
}

/**
 * Explicitly removes a texture from the collection.  Returns true if the
 * texture was removed, false if it wasn't there or if there was some error.
 */
bool EggTextureCollection::
remove_texture(EggTexture *texture) {
  nassertr(_textures.size() == _ordered_textures.size(), false);

  Textures::iterator ti;
  ti = _textures.find(texture);
  if (ti == _textures.end()) {
    // This texture is not a member of the collection.
    return false;
  }

  _textures.erase(ti);

  OrderedTextures::iterator oti;
  PT_EggTexture ptex = texture;
  oti = find(_ordered_textures.begin(), _ordered_textures.end(), ptex);
  nassertr(oti != _ordered_textures.end(), false);

  _ordered_textures.erase(oti);

  nassertr(_textures.size() == _ordered_textures.size(), false);
  return true;
}

/**
 * Creates a new texture if there is not already one equivalent (according to
 * eq, see EggTexture::is_equivalent_to()) to the indicated texture, or
 * returns the existing one if there is.
 */
EggTexture *EggTextureCollection::
create_unique_texture(const EggTexture &copy, int eq) {
  // This requires a complete linear traversal, not terribly efficient.
  OrderedTextures::const_iterator oti;
  for (oti = _ordered_textures.begin();
       oti != _ordered_textures.end();
       ++oti) {
    EggTexture *tex = (*oti);
    if (copy.is_equivalent_to(*tex, eq)) {
      // cout << "tex:" << tex->get_name() << "---copy:" << copy.get_name() <<
      // endl;
      return tex;
    }
  }
  // cout << "adding a texture to collection: " << copy.get_name() << endl;
  EggTexture *new_texture = new EggTexture(copy);
  add_texture(new_texture);
  return new_texture;
}

/**
 * Returns the texture with the indicated TRef name, or NULL if no texture
 * matches.
 */
EggTexture *EggTextureCollection::
find_tref(const std::string &tref_name) const {
  // This requires a complete linear traversal, not terribly efficient.
  OrderedTextures::const_iterator oti;
  for (oti = _ordered_textures.begin();
       oti != _ordered_textures.end();
       ++oti) {
    EggTexture *tex = (*oti);
    if (tex->get_name() == tref_name) {
      return tex;
    }
  }

  return nullptr;
}

/**
 * Returns the texture with the indicated filename, or NULL if no texture
 * matches.
 */
EggTexture *EggTextureCollection::
find_filename(const Filename &filename) const {
  // This requires a complete linear traversal, not terribly efficient.
  OrderedTextures::const_iterator oti;
  for (oti = _ordered_textures.begin();
       oti != _ordered_textures.end();
       ++oti) {
    EggTexture *tex = (*oti);
    if (tex->get_filename() == filename) {
      return tex;
    }
  }

  return nullptr;
}
