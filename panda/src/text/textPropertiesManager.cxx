/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textPropertiesManager.cxx
 * @author drose
 * @date 2004-04-07
 */

#include "textPropertiesManager.h"
#include "indent.h"

using std::string;

TextPropertiesManager *TextPropertiesManager::_global_ptr = nullptr;

/**
 * The constructor is not intended to be called directly; there is only one
 * TextPropertiesManager and it constructs itself.  This could have been a
 * private constructor, but gcc issues a spurious warning if the constructor
 * is private and the class has no friends.
 */
TextPropertiesManager::
TextPropertiesManager() {
}

/**
 * Don't call the destructor.
 */
TextPropertiesManager::
~TextPropertiesManager() {
}

/**
 * Defines the TextProperties associated with the indicated name.  When the
 * name is subsequently encountered in text embedded between \1 characters in
 * a TextNode string, the following text will be rendered with these
 * properties.
 *
 * If there was already a TextProperties structure associated with this name,
 * it is quietly replaced with the new definition.
 */
void TextPropertiesManager::
set_properties(const string &name, const TextProperties &properties) {
  _properties[name] = properties;
}

/**
 * Returns the TextProperties associated with the indicated name.  If there
 * was not previously a TextProperties associated with this name, a warning is
 * printed and then a default TextProperties structure is associated with the
 * name, and returned.
 *
 * Call has_properties() instead to check whether a particular name has been
 * defined.
 */
TextProperties TextPropertiesManager::
get_properties(const string &name) {
  Properties::const_iterator pi;
  pi = _properties.find(name);
  if (pi != _properties.end()) {
    return (*pi).second;
  }

  text_cat.warning()
    << "Creating default TextProperties for name '" << name << "'\n";

  TextProperties default_properties;
  _properties[name] = default_properties;
  return default_properties;
}

/**
 * Returns true if a TextProperties structure has been associated with the
 * indicated name, false otherwise.  Normally this means set_properties() has
 * been called with this name, but because get_properties() will implicitly
 * create a default TextProperties structure, it may also mean simply that
 * get_properties() has been called with the indicated name.
 */
bool TextPropertiesManager::
has_properties(const string &name) const {
  Properties::const_iterator pi;
  pi = _properties.find(name);
  return (pi != _properties.end());
}

/**
 * Removes the named TextProperties structure from the manager.
 */
void TextPropertiesManager::
clear_properties(const string &name) {
  _properties.erase(name);
}

/**
 * Defines the TextGraphic associated with the indicated name.  When the name
 * is subsequently encountered in text embedded between \5 characters in a
 * TextNode string, the specified graphic will be embedded in the text at that
 * point.
 *
 * If there was already a TextGraphic structure associated with this name, it
 * is quietly replaced with the new definition.
 */
void TextPropertiesManager::
set_graphic(const string &name, const TextGraphic &graphic) {
  _graphics[name] = graphic;
}

/**
 * This flavor of set_graphic implicitly creates a frame for the model using
 * the model's actual computed bounding volume, as derived from
 * NodePath::calc_tight_bounds().  Create a TextGraphic object first if you
 * want to have explicit control of the frame.
 */
void TextPropertiesManager::
set_graphic(const string &name, const NodePath &model) {
  LPoint3 min_point, max_point;
  model.calc_tight_bounds(min_point, max_point);

  TextGraphic graphic(model,
                      min_point.dot(LVector3::right()),
                      max_point.dot(LVector3::right()),
                      min_point.dot(LVector3::up()),
                      max_point.dot(LVector3::up()));

  _graphics[name] = graphic;
}

/**
 * Returns the TextGraphic associated with the indicated name.  If there was
 * not previously a TextGraphic associated with this name, a warning is
 * printed and then a default TextGraphic structure is associated with the
 * name, and returned.
 *
 * Call has_graphic() instead to check whether a particular name has been
 * defined.
 */
TextGraphic TextPropertiesManager::
get_graphic(const string &name) {
  Graphics::const_iterator pi;
  pi = _graphics.find(name);
  if (pi != _graphics.end()) {
    return (*pi).second;
  }

  text_cat.warning()
    << "Creating default TextGraphic for name '" << name << "'\n";

  TextGraphic default_graphic;
  _graphics[name] = default_graphic;
  return default_graphic;
}

/**
 * Returns true if a TextGraphic structure has been associated with the
 * indicated name, false otherwise.  Normally this means set_graphic() has
 * been called with this name, but because get_graphic() will implicitly
 * create a default TextGraphic structure, it may also mean simply that
 * get_graphic() has been called with the indicated name.
 */
bool TextPropertiesManager::
has_graphic(const string &name) const {
  Graphics::const_iterator pi;
  pi = _graphics.find(name);
  return (pi != _graphics.end());
}

/**
 * Removes the named TextGraphic structure from the manager.
 */
void TextPropertiesManager::
clear_graphic(const string &name) {
  _graphics.erase(name);
}

/**
 *
 */
void TextPropertiesManager::
write(std::ostream &out, int indent_level) const {
  Properties::const_iterator pi;
  for (pi = _properties.begin(); pi != _properties.end(); ++pi) {
    indent(out, indent_level)
      << "TextProperties " << (*pi).first << ":\n";
    (*pi).second.write(out, indent_level + 2);
  }
}

/**
 * Returns the pointer to the global TextPropertiesManager object.
 */
TextPropertiesManager *TextPropertiesManager::
get_global_ptr() {
  if (_global_ptr == nullptr) {
    _global_ptr = new TextPropertiesManager;
  }
  return _global_ptr;
}

/**
 * Returns a pointer to the TextProperties with the indicated name, or NULL if
 * there is no properties with that name.
 */
const TextProperties *TextPropertiesManager::
get_properties_ptr(const string &name) {
  Properties::const_iterator pi;
  pi = _properties.find(name);
  if (pi != _properties.end()) {
    return &(*pi).second;
  }
  return nullptr;
}

/**
 * Returns a pointer to the TextGraphic with the indicated name, or NULL if
 * there is no graphic with that name.
 */
const TextGraphic *TextPropertiesManager::
get_graphic_ptr(const string &name) {
  Graphics::const_iterator pi;
  pi = _graphics.find(name);
  if (pi != _graphics.end()) {
    return &(*pi).second;
  }
  return nullptr;
}
