// Filename: textFont.cxx
// Created by:  drose (03May01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "textFont.h"
#include "config_text.h"

#include <geom.h>
#include <geomPoint.h>
#include <geomNode.h>
#include <namedNode.h>
#include <renderRelation.h>

TypeHandle TextFont::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: isblank
//  Description: An internal function, similar to isspace(), except it
//               does not consider newlines to be whitespace.
////////////////////////////////////////////////////////////////////
INLINE bool
isblank(char ch) {
  return (ch == ' ' || ch == '\t');
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::CharDef::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TextFont::CharDef::
CharDef(Geom *geom, float width, const AllTransitionsWrapper &trans) :
  _geom(geom), _width(width), _trans(trans) { }

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Constructor
//       Access: Published
//  Description: The constructor expects the root node to a model
//               generated via egg-mkfont, which consists of a set of
//               models, one per each character in the font.
////////////////////////////////////////////////////////////////////
TextFont::
TextFont(Node *font_def) {
  nassertv(font_def != (Node *)NULL);
  _font = font_def;
  _defs.clear();
  _font_height = 1.0;

  find_characters(font_def);

  if (_font->is_of_type(NamedNode::get_class_type())) {
    NamedNode *named_node = DCAST(NamedNode, _font);
    set_name(named_node->get_name());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
TextFont::
~TextFont() {
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::calc_width
//       Access: Published
//  Description: Returns the width of a single character of the font,
//               or 0.0 if the character is not known.
////////////////////////////////////////////////////////////////////
float TextFont::
calc_width(char ch) const {
  if (ch == ' ') {
    // A space is a special case.
    return 0.25;
  }

  CharDefs::const_iterator cdi = _defs.find(ch);
  if (cdi == _defs.end()) {
    // Unknown character.
    return 0.0;
  }

  return (*cdi).second._width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::calc_width
//       Access: Published
//  Description: Returns the width of a line of text of arbitrary
//               characters.  The line should not include the newline
//               character.
////////////////////////////////////////////////////////////////////
float TextFont::
calc_width(const string &line) const {
  float width = 0.0;

  string::const_iterator si;
  for (si = line.begin(); si != line.end(); ++si) {
    width += calc_width(*si);
  }

  return width;
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::wordwrap_to
//       Access: Published
//  Description: Inserts newlines into the given text at the
//               appropriate places in order to make each line be the
//               longest possible line that is not longer than
//               wordwrap_width (and does not break any words, if
//               possible).  Returns the new string.
////////////////////////////////////////////////////////////////////
string TextFont::
wordwrap_to(const string &text, float wordwrap_width) const {
  string output_text;

  size_t p = 0;

  // Preserve any initial whitespace and newlines.
  while (p < text.length() && isspace(text[p])) {
    output_text += text[p];
    p++;
  }
  bool first_line = true;

  while (p < text.length()) {
    nassertr(!isspace(text[p]), "");

    // Scan the next n characters, until the end of the string or an
    // embedded newline character, or we exceed wordwrap_width.

    size_t q = p;
    bool any_spaces = false;

    float width = 0.0;
    while (q < text.length() && text[q] != '\n' && width <= wordwrap_width) {
      if (isspace(text[q])) {
        any_spaces = true;
      }

      width += calc_width(text[q]);
      q++;
    }

    if (q < text.length() && any_spaces) {
      // If we stopped because we exceeded the wordwrap width, then
      // back up to the end of the last complete word.

      while (q > p && !isspace(text[q])) {
        q--;
      }
    }

    // Skip additional whitespace between the lines.
    size_t next_start = q;
    while (next_start < text.length() && isblank(text[next_start])) {
      next_start++;
    }

    // Trim off any more blanks on the end.
    while (q > p && isspace(text[q - 1])) {
      q--;
    }

    if (next_start == p) {
      // No characters got in at all.  This could only happen if the
      // wordwrap width is narrower than a single character.
      q++;
      next_start++;
      while (next_start < text.length() && isblank(text[next_start])) {
        next_start++;
      }
    }

    if (!first_line) {
      output_text += '\n';
    }
    first_line = false;
    output_text += text.substr(p, q - p);

    // Now prepare to wrap the next line.

    if (next_start < text.length() && text[next_start] == '\n') {
      // Skip a single embedded newline.
      next_start++;
    }
    p = next_start;

    // Preserve any initial whitespace and newlines.
    while (p < text.length() && isspace(text[p])) {
      output_text += text[p];
      p++;
    }
  }

  return output_text;
}


////////////////////////////////////////////////////////////////////
//     Function: TextFont::write
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void TextFont::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << "TextFont " << get_name() << "; "
    << _defs.size() << " characters available in font.\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::find_character_gsets
//       Access: Private
//  Description: Given that 'root' is a Node containing at least a
//               polygon and a point which define the character's
//               appearance and kern position, respectively,
//               recursively walk the hierarchy and root and locate
//               those two Geoms.
////////////////////////////////////////////////////////////////////
bool TextFont::
find_character_gsets(Node *root, Geom *&ch, GeomPoint *&dot,
                     AllTransitionsWrapper &trans) {
  if (root->is_of_type(GeomNode::get_class_type())) {
    GeomNode *geode = (GeomNode *)root;

    bool found = false;
    for (int i = 0; i < geode->get_num_geoms(); i++) {
      dDrawable *geom = geode->get_geom(i);
      if (geom->is_of_type(GeomPoint::get_class_type())) {
        dot = DCAST(GeomPoint, geom);

      } else if (geom->is_of_type(Geom::get_class_type())) {
        ch = DCAST(Geom, geom);
        found = true;
      }
    }
    return found;

  } else {
    TypeHandle graph_type = RenderRelation::get_class_type();
    int num_children = root->get_num_children(graph_type);
    for (int i = 0; i < num_children; i++) {
      NodeRelation *child_arc = root->get_child(graph_type, i);
      if (find_character_gsets(child_arc->get_child(), ch, dot, trans)) {
        trans.extract_from(child_arc);
      }
    }
    return false;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TextFont::find_characters
//       Access: Private
//  Description: Walk the hierarchy beginning at the indicated root
//               and locate any nodes whose names are just integers.
//               These are taken to be characters, and their
//               definitions and kern informations are retrieved.
////////////////////////////////////////////////////////////////////
void TextFont::
find_characters(Node *root) {
  string name;
  if (root->is_of_type(NamedNode::get_class_type())) {
    name = DCAST(NamedNode, root)->get_name();
  }

  bool all_digits = !name.empty();
  const char *p = name.c_str();
  while (all_digits && *p != '\0') {
    // VC++ complains if we treat an int as a bool, so we have to do
    // this != 0 comparsion on the int isdigit() function to shut it
    // up.
    all_digits = (isdigit(*p) != 0);
    p++;
  }

  if (all_digits) {
    int character = atoi(name.c_str());
    Geom *ch = NULL;
    GeomPoint *dot = NULL;
    AllTransitionsWrapper trans;
    find_character_gsets(root, ch, dot, trans);
    if (dot != NULL) {
      // Get the first vertex from the "dot" geoset.  This will be the
      // origin of the next character.
      PTA_Vertexf alist;
      PTA_ushort ilist;
      GeomBindType bind;
      float width;
      dot->get_coords(alist, bind, ilist);
      if (ilist.empty()) {
        width = alist[0][0];
      } else {
        width = alist[ilist[0]][0];
      }

      _defs[character] = CharDef(ch, width, trans);
    }

  } else if (name == "ds") {
    // The group "ds" is a special node that indicate's the font's
    // design size, or line height.

    Geom *ch = NULL;
    GeomPoint *dot = NULL;
    AllTransitionsWrapper trans;
    find_character_gsets(root, ch, dot, trans);
    if (dot != NULL) {
      // Get the first vertex from the "dot" geoset.  This will be the
      // design size indicator.
      PTA_Vertexf alist;
      PTA_ushort ilist;
      GeomBindType bind;
      dot->get_coords(alist, bind, ilist);
      if (ilist.empty()) {
        _font_height = alist[0][2];
      } else {
        _font_height = alist[ilist[0]][2];
      }
    }

  } else {
    TypeHandle graph_type = RenderRelation::get_class_type();
    int num_children = root->get_num_children(graph_type);
    for (int i = 0; i < num_children; i++) {
      NodeRelation *child_arc = root->get_child(graph_type, i);
      find_characters(child_arc->get_child());
    }
  }
}
