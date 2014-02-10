// Filename: textureCollection.cxx
// Created by:  drose (16Mar02)
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

#include "textureCollection.h"
#include "indent.h"

#ifdef HAVE_PYTHON
#include "py_panda.h"
#endif

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextureCollection::
TextureCollection() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextureCollection::
TextureCollection(const TextureCollection &copy) :
  _textures(copy._textures)
{
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::Copy Assignment Operator
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void TextureCollection::
operator = (const TextureCollection &copy) {
  _textures = copy._textures;
}

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::Constructor
//       Access: Published
//  Description: This special constructor accepts a Python list of
//               Textures.  Since this constructor accepts a generic
//               PyObject *, it should be the last constructor listed
//               in the class record.
////////////////////////////////////////////////////////////////////
TextureCollection::
TextureCollection(PyObject *self, PyObject *sequence) {
  // We have to pre-initialize self's "this" pointer when we receive
  // self in the constructor--the caller can't initialize this for us.
  ((Dtool_PyInstDef *)self)->_ptr_to_object = this;

  if (!PySequence_Check(sequence)) {
    // If passed with a non-sequence, this isn't the right constructor.
    PyErr_SetString(PyExc_TypeError, "TextureCollection constructor requires a sequence");
    return;
  }

  int size = PySequence_Size(sequence);
  for (int i = 0; i < size; ++i) {
    PyObject *item = PySequence_GetItem(sequence, i);
    if (item == NULL) {
      return;
    }
    PyObject *result = PyObject_CallMethod(self, (char *)"add_texture", (char *)"O", item);
    Py_DECREF(item);
    if (result == NULL) {
      // Unable to add item--probably it wasn't of the appropriate type.
      ostringstream stream;
      stream << "Element " << i << " in sequence passed to TextureCollection constructor could not be added";
      string str = stream.str();
      PyErr_SetString(PyExc_TypeError, str.c_str());
      return;
    }
    Py_DECREF(result);
  }
}
#endif  // HAVE_PYTHON

#ifdef HAVE_PYTHON
////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::__reduce__
//       Access: Published
//  Description: This special Python method is implement to provide
//               support for the pickle module.
////////////////////////////////////////////////////////////////////
PyObject *TextureCollection::
__reduce__(PyObject *self) const {
  // Here we will return a 4-tuple: (Class, (args), None, iterator),
  // where iterator is an iterator that will yield successive
  // Textures.

  // We should return at least a 2-tuple, (Class, (args)): the
  // necessary class object whose constructor we should call
  // (e.g. this), and the arguments necessary to reconstruct this
  // object.

  PyObject *this_class = PyObject_Type(self);
  if (this_class == NULL) {
    return NULL;
  }

  // Since a TextureCollection is itself an iterator, we can simply
  // pass it as the fourth tuple component.
  PyObject *result = Py_BuildValue("(O()OO)", this_class, Py_None, self);
  Py_DECREF(this_class);
  return result;
}
#endif  // HAVE_PYTHON

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::add_texture
//       Access: Published
//  Description: Adds a new Texture to the collection.
////////////////////////////////////////////////////////////////////
void TextureCollection::
add_texture(Texture *texture) {
  // If the pointer to our internal array is shared by any other
  // TextureCollections, we have to copy the array now so we won't
  // inadvertently modify any of our brethren TextureCollection
  // objects.

  if (_textures.get_ref_count() > 1) {
    Textures old_textures = _textures;
    _textures = Textures::empty_array(0);
    _textures.v() = old_textures.v();
  }

  _textures.push_back(texture);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::remove_texture
//       Access: Published
//  Description: Removes the indicated Texture from the collection.
//               Returns true if the texture was removed, false if it was
//               not a member of the collection.
////////////////////////////////////////////////////////////////////
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
  // inadvertently modify any of our brethren TextureCollection
  // objects.

  if (_textures.get_ref_count() > 1) {
    Textures old_textures = _textures;
    _textures = Textures::empty_array(0);
    _textures.v() = old_textures.v();
  }

  _textures.erase(_textures.begin() + texture_index);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::add_textures_from
//       Access: Published
//  Description: Adds all the Textures indicated in the other
//               collection to this texture.  The other textures are simply
//               appended to the end of the textures in this list;
//               duplicates are not automatically removed.
////////////////////////////////////////////////////////////////////
void TextureCollection::
add_textures_from(const TextureCollection &other) {
  int other_num_textures = other.get_num_textures();
  for (int i = 0; i < other_num_textures; i++) {
    add_texture(other.get_texture(i));
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::remove_textures_from
//       Access: Published
//  Description: Removes from this collection all of the Textures
//               listed in the other collection.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::remove_duplicate_textures
//       Access: Published
//  Description: Removes any duplicate entries of the same Textures
//               on this collection.  If a Texture appears multiple
//               times, the first appearance is retained; subsequent
//               appearances are removed.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::has_texture
//       Access: Published
//  Description: Returns true if the indicated Texture appears in
//               this collection, false otherwise.
////////////////////////////////////////////////////////////////////
bool TextureCollection::
has_texture(Texture *texture) const {
  for (int i = 0; i < get_num_textures(); i++) {
    if (texture == get_texture(i)) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::clear
//       Access: Published
//  Description: Removes all Textures from the collection.
////////////////////////////////////////////////////////////////////
void TextureCollection::
clear() {
  _textures.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::find_texture
//       Access: Published
//  Description: Returns the texture in the collection with the
//               indicated name, if any, or NULL if no texture has
//               that name.
////////////////////////////////////////////////////////////////////
Texture *TextureCollection::
find_texture(const string &name) const {
  int num_textures = get_num_textures();
  for (int i = 0; i < num_textures; i++) {
    Texture *texture = get_texture(i);
    if (texture->get_name() == name) {
      return texture;
    }
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::get_num_textures
//       Access: Published
//  Description: Returns the number of Textures in the collection.
////////////////////////////////////////////////////////////////////
int TextureCollection::
get_num_textures() const {
  return _textures.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::get_texture
//       Access: Published
//  Description: Returns the nth Texture in the collection.
////////////////////////////////////////////////////////////////////
Texture *TextureCollection::
get_texture(int index) const {
  nassertr(index >= 0 && index < (int)_textures.size(), NULL);

  return _textures[index];
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::operator []
//       Access: Published
//  Description: Returns the nth Texture in the collection.  This is
//               the same as get_texture(), but it may be a more
//               convenient way to access it.
////////////////////////////////////////////////////////////////////
Texture *TextureCollection::
operator [] (int index) const {
  nassertr(index >= 0 && index < (int)_textures.size(), NULL);

  return _textures[index];
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::size
//       Access: Published
//  Description: Returns the number of textures in the collection.  This
//               is the same thing as get_num_textures().
////////////////////////////////////////////////////////////////////
int TextureCollection::
size() const {
  return _textures.size();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::output
//       Access: Published
//  Description: Writes a brief one-line description of the
//               TextureCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void TextureCollection::
output(ostream &out) const {
  if (get_num_textures() == 1) {
    out << "1 Texture";
  } else {
    out << get_num_textures() << " Textures";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextureCollection::write
//       Access: Published
//  Description: Writes a complete multi-line description of the
//               TextureCollection to the indicated output stream.
////////////////////////////////////////////////////////////////////
void TextureCollection::
write(ostream &out, int indent_level) const {
  for (int i = 0; i < get_num_textures(); i++) {
    indent(out, indent_level) << *get_texture(i) << "\n";
  }
}
