// Filename: textPropertiesManager.h
// Created by:  drose (07Apr04)
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

#ifndef TEXTPROPERTIESMANAGER_H
#define TEXTPROPERTIESMANAGER_H

#include "pandabase.h"

#include "config_text.h"
#include "textProperties.h"

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
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TextPropertiesManager {
protected:
  TextPropertiesManager();
  ~TextPropertiesManager();

PUBLISHED:
  void set_properties(const string &name, const TextProperties &properties);
  TextProperties get_properties(const string &name);
  bool has_properties(const string &name) const;
  void clear_properties(const string &name);

  void write(ostream &out, int indent_level = 0) const;

  static TextPropertiesManager *get_global_ptr();

private:
  typedef pmap<string, TextProperties> Properties;
  Properties _properties;

  static TextPropertiesManager *_global_ptr;
};

#include "textPropertiesManager.I"

#endif
