// Filename: textPropertiesManager.h
// Created by:  drose (07Apr04)
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

#ifndef TEXTPROPERTIESMANAGER_H
#define TEXTPROPERTIESMANAGER_H

#include "pandabase.h"

#include "config_text.h"
#include "textProperties.h"
#include "textGraphic.h"

////////////////////////////////////////////////////////////////////
//       Class : TextPropertiesManager
// Description : This defines all of the TextProperties structures
//               that might be referenced by name from an embedded
//               text string.
//
//               A text string, as rendered by a TextNode, can contain
//               embedded references to one of the TextProperties
//               defined here, by enclosing the name between \1 (ASCII
//               0x01) characters; this causes a "push" to the named
//               state.  All text following the closing \1 character
//               will then be rendered in the new state.  The next \2
//               (ASCII 0x02) character will then restore the previous
//               state for subsequent text.
//
//               For instance, "x\1up\1n\2 + y" indicates that the
//               character "x" will be rendered in the normal state,
//               the character "n" will be rendered in the "up" state,
//               and then " + y" will be rendered in the normal state
//               again.
//
//               This can also be used to define arbitrary models that
//               can serve as embedded graphic images in a text
//               paragraph.  This works similarly; the convention is
//               to create a TextGraphic that describes the graphic
//               image, and then associate it here via the
//               set_graphic() call.  Then "\5name\5" will embed the
//               named graphic.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TEXT TextPropertiesManager {
protected:
  TextPropertiesManager();
  ~TextPropertiesManager();

PUBLISHED:
  void set_properties(const string &name, const TextProperties &properties);
  TextProperties get_properties(const string &name);
  bool has_properties(const string &name) const;
  void clear_properties(const string &name);

  void set_graphic(const string &name, const TextGraphic &graphic);
  void set_graphic(const string &name, const NodePath &model);
  TextGraphic get_graphic(const string &name);
  bool has_graphic(const string &name) const;
  void clear_graphic(const string &name);

  void write(ostream &out, int indent_level = 0) const;

  static TextPropertiesManager *get_global_ptr();

public:
  const TextProperties *get_properties_ptr(const string &name);
  const TextGraphic *get_graphic_ptr(const string &name);

private:
  typedef pmap<string, TextProperties> Properties;
  Properties _properties;

  typedef pmap<string, TextGraphic> Graphics;
  Graphics _graphics;

  static TextPropertiesManager *_global_ptr;
};

#include "textPropertiesManager.I"

#endif
